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

#include "TEdge.h"
#include "Int128.h"
#include "Util.h"
#include "Clipper.h"

#include <cmath>

using namespace ClipperLib;


TEdge::TEdge(TEdge *next, TEdge *prev, const IntPoint &pt, PolyType polyType) :
  xcurr(pt.X), ycurr(pt.Y), polyType(polyType), next(next), prev(prev) {

  if (next->ycurr <= ycurr) {
    xbot = xcurr;
    ybot = ycurr;
    xtop = next->xcurr;
    ytop = next->ycurr;
    windDelta = 1;

  } else {
    xtop = xcurr;
    ytop = ycurr;
    xbot = next->xcurr;
    ybot = next->ycurr;
    windDelta = -1;
  }

  deltaX = xtop - xbot;
  deltaY = ytop - ybot;
  if (deltaY == 0) dx = CLIPPER_HORIZONTAL;
  else dx = (double)(deltaX) / deltaY;

  outIdx = -1;
}


bool TEdge::IntersectPoint(TEdge &edge2, IntPoint &ip,
                           bool UseFullInt64Range) const {
  double b1, b2;

  if (SlopesEqual(edge2, UseFullInt64Range)) {
    if (edge2.ybot > ybot) ip.Y = edge2.ybot;
    else ip.Y = ybot;

    return false;
  }

  if (CLIPPER_NEAR_ZERO(dx)) {
    ip.X = xbot;
    if (CLIPPER_NEAR_EQUAL(edge2.dx, CLIPPER_HORIZONTAL)) ip.Y = edge2.ybot;
    else {
      b2 = edge2.ybot - (edge2.xbot / edge2.dx);
      ip.Y = Round(ip.X / edge2.dx + b2);
    }

  } else if (CLIPPER_NEAR_ZERO(edge2.dx)) {
    ip.X = edge2.xbot;

    if (CLIPPER_NEAR_EQUAL(dx, CLIPPER_HORIZONTAL)) ip.Y = ybot;
    else {
      b1 = ybot - (xbot / dx);
      ip.Y = Round(ip.X / dx + b1);
    }

  } else {
    b1 = xbot - ybot * dx;
    b2 = edge2.xbot - edge2.ybot * edge2.dx;

    double q = (b2 - b1) / (dx - edge2.dx);
    ip.Y = Round(q);

    if (std::fabs(dx) < std::fabs(edge2.dx))
      ip.X = Round(dx * q + b1);
    else ip.X = Round(edge2.dx * q + b2);
  }

  if (ip.Y < ytop || ip.Y < edge2.ytop) {
    if (ytop > edge2.ytop) {
      ip.X = xtop;
      ip.Y = ytop;

      return edge2.TopX(ytop) < xtop;
    }

    ip.X = edge2.xtop;
    ip.Y = edge2.ytop;

    return TopX(edge2.ytop) > edge2.xtop;
  }

  return true;
}


bool TEdge::SlopesEqual(const TEdge &e2, bool UseFullInt64Range) const {
  if (UseFullInt64Range)
    return Int128Mul(deltaY, e2.deltaX) == Int128Mul(deltaX, e2.deltaY);

  return deltaY * e2.deltaX == deltaX * e2.deltaY;
}
