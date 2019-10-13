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

#include "OutPt.h"
#include "Int128.h"

#include <cmath>

using namespace ClipperLib;


void OutPt::Dispose() {
  prev->next = 0;
  OutPt *pp = this;

  while (pp) {
    OutPt *tmp = pp;
    pp = pp->next;
    delete tmp;
  }
}


bool OutPt::PointIsVertex(const IntPoint &pt) const {
  const OutPt *pp = this;

  do {
    if (pp->pt.Equal(pt)) return true;
    pp = pp->next;
  } while (pp != this);

  return false;
}


bool OutPt::PointOnPolygon(const IntPoint pt, bool UseFullInt64Range) const {
  const OutPt *pp = this;

  while (true) {
    if (pt.OnLineSegment(pp->pt, pp->next->pt, UseFullInt64Range))
      return true;

    pp = pp->next;
    if (pp == this) break;
  }

  return false;
}


bool OutPt::PointInPolygon(const IntPoint &pt, bool UseFullInt64Range) const {
  const OutPt *pp = this;
  bool result = false;

  if (UseFullInt64Range) {
    do {
      if ((((pp->pt.Y <= pt.Y) && (pt.Y < pp->prev->pt.Y)) ||
           ((pp->prev->pt.Y <= pt.Y) && (pt.Y < pp->pt.Y))) &&
          Int128(pt.X - pp->pt.X) <
          Int128Mul(pp->prev->pt.X - pp->pt.X, pt.Y - pp->pt.Y) /
          Int128(pp->prev->pt.Y - pp->pt.Y))
        result = !result;
      pp = pp->next;

    } while (pp != this);

  } else {
    do {
      if ((((pp->pt.Y <= pt.Y) && (pt.Y < pp->prev->pt.Y)) ||
           ((pp->prev->pt.Y <= pt.Y) && (pt.Y < pp->pt.Y))) &&
          (pt.X < (pp->prev->pt.X - pp->pt.X) * (pt.Y - pp->pt.Y) /
           (pp->prev->pt.Y - pp->pt.Y) + pp->pt.X)) result = !result;
      pp = pp->next;
    } while (pp != this);
  }

  return result;
}


void OutPt::ReversePolyPtLinks() {
  OutPt *pp1 = this;
  OutPt *pp2;

  do {
    pp2 = pp1->next;
    pp1->next = pp1->prev;
    pp1->prev = pp2;
    pp1 = pp2;

  } while (pp1 != this);
}


bool OutPt::FirstIsBottomPt(const OutPt *btmPt2) const {
  OutPt *p = prev;

  while (p->pt.Equal(pt) && (p != this)) p = p->prev;

  double dx1p = std::fabs(pt.GetDx(p->pt));
  p = next;

  while (p->pt.Equal(pt) && (p != this)) p = p->next;

  double dx1n = std::fabs(pt.GetDx(p->pt));
  p = btmPt2->prev;

  while (p->pt.Equal(btmPt2->pt) && (p != btmPt2)) p = p->prev;

  double dx2p = std::fabs(btmPt2->pt.GetDx(p->pt));
  p = btmPt2->next;

  while (p->pt.Equal(btmPt2->pt) && (p != btmPt2)) p = p->next;

  double dx2n = std::fabs(btmPt2->pt.GetDx(p->pt));

  return (dx1p >= dx2p && dx1p >= dx2n) || (dx1n >= dx2p && dx1n >= dx2n);
}


OutPt *OutPt::GetBottomPt() {
  OutPt *dups = 0;
  OutPt *pp = this;
  OutPt *p = next;

  while (p != pp) {
    if (p->pt.Y > pp->pt.Y) {
      pp = p;
      dups = 0;

    } else if (p->pt.Y == pp->pt.Y && p->pt.X <= pp->pt.X) {
      if (p->pt.X < pp->pt.X) {
        dups = 0;
        pp = p;
      } else if (p->next != pp && p->prev != pp) dups = p;
    }

    p = p->next;
  }

  if (dups) {
    // there appears to be at least 2 vertices at bottomPt so
    while (dups != p) {
      if (!p->FirstIsBottomPt(dups)) pp = dups;
      dups = dups->next;

      while (!dups->pt.Equal(pp->pt)) dups = dups->next;
    }
  }

  return pp;
}


OutPt *OutPt::FindSegment(bool UseFullInt64Range, IntPoint &pt1,
                          IntPoint &pt2) {
  // outPt1 & outPt2 => the overlap segment (if the function returns true)
  OutPt *pp = this;
  OutPt *pp2 = pp;
  IntPoint pt1a = pt1, pt2a = pt2;

  do {
    if (SlopesEqual(pt1a, pt2a, pp->pt, pp->prev->pt, UseFullInt64Range) &&
        SlopesEqual(pt1a, pt2a, pp->pt, UseFullInt64Range) &&
        GetOverlapSegment(pt1a, pt2a, pp->pt, pp->prev->pt, pt1, pt2))
      return pp;
    pp = pp->next;

  } while (pp != pp2);

  return 0;
}


OutPt *OutPt::InsertPolyPtBetween(OutPt *p2, const IntPoint pt) {
  if (this == p2) throw "JoinError";

  OutPt *result = new OutPt;
  result->pt = pt;

  if (p2 == next) {
    next = result;
    p2->prev = result;
    result->next = p2;
    result->prev = this;

  } else {
    p2->next = result;
    prev = result;
    result->next = this;
    result->prev = p2;
  }

  return result;
}


int OutPt::PointCount() const {
  int result = 0;
  const OutPt *p = this;

  do {
    result++;
    p = p->next;
  } while (p != this);

  return result;
}


bool OutPt::Contains(const OutPt *outPt2, bool UseFullInt64Range) const {
  const OutPt *pt = this;
  // Because the polygons may be touching, we need to find a vertex that
  // isn't touching the other polygon

  if (outPt2->PointOnPolygon(pt->pt, UseFullInt64Range)) {
    pt = pt->next;

    while (pt != this && outPt2->PointOnPolygon(pt->pt, UseFullInt64Range))
      pt = pt->next;

    if (pt == this) return true;
  }

  return outPt2->PointInPolygon(pt->pt, UseFullInt64Range);
}
