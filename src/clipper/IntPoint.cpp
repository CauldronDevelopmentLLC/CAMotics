/*******************************************************************************
 *                                                                             *
 * Author    :  Angus Johnson                                                  *
 * Version   :  5.1.6                                                          *
 * Date      :  23 May 2013                                                    *
 * Website   :  http://www.angusj.com                                          *
 * Copyright :  Angus Johnson 2010-2013                                        *
 *                                                                             *
 * License:                                                                    *
 * Use, modification & distribution is subject to Boost Software License Ver 1.*
 * http://www.boost.org/LICENSE_1_0.txt                                        *
 *                                                                             *
 * Attributions:                                                               *
 * The code in this library is an extension of Bala Vatti's clipping algorithm:*
 * "A generic solution to polygon clipping"                                    *
 * Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.            *
 * http://portal.acm.org/citation.cfm?id=129906                                *
 *                                                                             *
 * Computer graphics and geometric modeling: implementation and algorithms     *
 * By Max K. Agoston                                                           *
 * Springer; 1 edition (January 4, 2005)                                       *
 * http://books.google.com/books?q=vatti+clipping+agoston                      *
 *                                                                             *
 * See also:                                                                   *
 * "Polygon Offsetting by Computing Winding Numbers"                           *
 * Paper no. DETC2005-85513 pp. 565-575                                        *
 * ASME 2005 International Design Engineering Technical Conferences            *
 * and Computers and Information in Engineering Conference (IDETC/CIE2005)     *
 * September 24-28, 2005 , Long Beach, California, USA                         *
 * http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf             *
 *                                                                             *
 ******************************************************************************/

#include "IntPoint.h"

#include "Int128.h"
#include "Clipper.h"

#include <cmath>

using namespace ClipperLib;


double IntPoint::GetDx(const IntPoint &pt2) const {
  return Y == pt2.Y ? CLIPPER_HORIZONTAL : (double)(pt2.X - X) / (pt2.Y - Y);
}


void IntPoint::RangeTest(int64_t &maxrange) const {
  if (Abs(X) > maxrange) {
    if (Abs(X) > CLIPPER_HI) throw "Coordinate exceeds range bounds.";
    maxrange = CLIPPER_HI;
  }

  if (Abs(Y) > maxrange) {
    if (Abs(Y) > CLIPPER_HI) throw "Coordinate exceeds range bounds.";
    maxrange = CLIPPER_HI;
  }
}



bool IntPoint::PointsAreClose(const IntPoint &pt2, double distSqrd) const {
  double dx = (double)X - pt2.X;
  double dy = (double)Y - pt2.Y;

  return (dx * dx) + (dy * dy) <= distSqrd;
}


bool IntPoint::UpdateBotPt(IntPoint &botPt) const {
  if (Y > botPt.Y || (Y == botPt.Y && X < botPt.X)) {
    botPt = *this;
    return true;
  }

  return false;
}


bool IntPoint::Equal(const IntPoint &pt2) const {
  return X == pt2.X && Y == pt2.Y;
}


void IntPoint::Swap(IntPoint &pt2) {
  IntPoint tmp = *this;
  *this = pt2;
  pt2 = tmp;
}


bool IntPoint::OnLineSegment(const IntPoint linePt1, const IntPoint linePt2,
                             bool UseFullInt64Range) const {
  if (UseFullInt64Range)
    return ((X == linePt1.X) && (Y == linePt1.Y)) ||
      ((X == linePt2.X) && (Y == linePt2.Y)) ||
      (((X > linePt1.X) == (X < linePt2.X)) &&
       ((Y > linePt1.Y) == (Y < linePt2.Y)) &&
       ((Int128Mul((X - linePt1.X), (linePt2.Y - linePt1.Y)) ==
         Int128Mul((linePt2.X - linePt1.X), (Y - linePt1.Y)))));


  return ((X == linePt1.X) && (Y == linePt1.Y)) ||
    ((X == linePt2.X) && (Y == linePt2.Y)) ||
    (((X > linePt1.X) == (X < linePt2.X)) &&
     ((Y > linePt1.Y) == (Y < linePt2.Y)) &&
     ((X - linePt1.X) * (linePt2.Y - linePt1.Y) ==
      (linePt2.X - linePt1.X) * (Y - linePt1.Y)));
}


DoublePoint IntPoint::GetUnitNormal(const IntPoint &pt2) const {
  if (pt2.X == X && pt2.Y == Y) return DoublePoint(0, 0);

  double dx = (double)(pt2.X - X);
  double dy = (double)(pt2.Y - Y);
  double f = 1 * 1.0 / std::sqrt(dx * dx + dy * dy);

  dx *= f;
  dy *= f;

  return DoublePoint(dy, -dx);
}


double IntPoint::DistanceSqrd(const IntPoint &pt2) const {
  double dx = (double)X - pt2.X;
  double dy = (double)Y - pt2.Y;

  return dx * dx + dy * dy;
}


DoublePoint IntPoint::ClosestPointOnLine(const IntPoint &linePt1,
                                         const IntPoint &linePt2) const {
  double dx = ((double)linePt2.X - linePt1.X);
  double dy = ((double)linePt2.Y - linePt1.Y);

  if (dx == 0 && dy == 0)
    return DoublePoint((double)linePt1.X, (double)linePt1.Y);

  double q = ((X - linePt1.X) * dx + (Y - linePt1.Y) * dy) /
    (dx * dx + dy * dy);

  return DoublePoint((1 - q) * linePt1.X + q * linePt2.X,
                     (1 - q) * linePt1.Y + q * linePt2.Y);
}


bool IntPoint::IsBetween(const IntPoint pt1, const IntPoint pt2) const {
  if (pt1.Equal(*this) || pt2.Equal(*this)) return true;
  else if (pt1.X != pt2.X) return (pt1.X < X) == (X < pt2.X);
  else return (pt1.Y < Y) == (Y < pt2.Y);
}


void IntPoint::write(std::ostream &stream) const {
  stream << X << ' ' << Y << '\n';
}


namespace ClipperLib {
  bool GetOverlapSegment(IntPoint pt1a, IntPoint pt1b, IntPoint pt2a,
                         IntPoint pt2b, IntPoint &pt1, IntPoint &pt2) {
    // precondition: segments are colinear.
    if (Abs(pt1a.X - pt1b.X) > Abs(pt1a.Y - pt1b.Y)) {
      if (pt1a.X > pt1b.X) pt1a.Swap(pt1b);
      if (pt2a.X > pt2b.X) pt2a.Swap(pt2b);
      if (pt1a.X > pt2a.X) pt1 = pt1a; else pt1 = pt2a;
      if (pt1b.X < pt2b.X) pt2 = pt1b; else pt2 = pt2b;
      return pt1.X < pt2.X;
    }

    if (pt1a.Y < pt1b.Y) pt1a.Swap(pt1b);
    if (pt2a.Y < pt2b.Y) pt2a.Swap(pt2b);
    if (pt1a.Y < pt2a.Y) pt1 = pt1a; else pt1 = pt2a;
    if (pt1b.Y > pt2b.Y) pt2 = pt1b; else pt2 = pt2b;

    return pt1.Y > pt2.Y;
  }


  bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3,
                   bool UseFullInt64Range) {
    if (UseFullInt64Range)
      return Int128Mul(pt1.Y - pt2.Y, pt2.X - pt3.X) ==
        Int128Mul(pt1.X - pt2.X, pt2.Y - pt3.Y);

    return (pt1.Y - pt2.Y) * (pt2.X - pt3.X) ==
      (pt1.X - pt2.X) * (pt2.Y - pt3.Y);
  }


  bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3,
                   const IntPoint pt4, bool UseFullInt64Range) {
    if (UseFullInt64Range)
      return Int128Mul(pt1.Y - pt2.Y, pt3.X - pt4.X) ==
        Int128Mul(pt1.X - pt2.X, pt3.Y - pt4.Y);

    return (pt1.Y - pt2.Y) * (pt3.X - pt4.X) ==
      (pt1.X - pt2.X) * (pt3.Y - pt4.Y);
  }


  bool SlopesNearColinear(const IntPoint &pt1, const IntPoint &pt2,
                          const IntPoint &pt3, double distSqrd) {
    if (pt1.DistanceSqrd(pt3) < pt1.DistanceSqrd(pt2)) return false;

    DoublePoint cpol = pt2.ClosestPointOnLine(pt1, pt3);
    double dx = pt2.X - cpol.X;
    double dy = pt2.Y - cpol.Y;

    return dx * dx + dy * dy < distSqrd;
  }
}
