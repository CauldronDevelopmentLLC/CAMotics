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

#pragma once

#include "Enum.h"
#include "IntPoint.h"
#include "Util.h"

#include <vector>

#include <stdint.h>


namespace ClipperLib {
  struct TEdge {
    int64_t xbot = 0;
    int64_t ybot = 0;
    int64_t xcurr = 0;
    int64_t ycurr = 0;
    int64_t xtop = 0;
    int64_t ytop = 0;

    double dx = 0;
    int64_t deltaX = 0;
    int64_t deltaY = 0;

    PolyType polyType = ptSubject;
    EdgeSide side = esNone;

    int windDelta = 0; // 1 or -1 depending on winding direction
    int windCnt = 0;
    int windCnt2 = 0;  // winding count of the opposite polytype
    int outIdx = 0;

    TEdge *next = 0;
    TEdge *prev = 0;
    TEdge *nextInLML = 0;
    TEdge *nextInAEL = 0;
    TEdge *prevInAEL = 0;
    TEdge *nextInSEL = 0;
    TEdge *prevInSEL = 0;

    TEdge() {}
    TEdge(TEdge *eNext, TEdge *ePrev, const IntPoint &pt, PolyType polyType);


    bool IsMinima() const {
      return prev->nextInLML != this && next->nextInLML != this;
    }


    bool IsMaxima(const int64_t Y) const {return ytop == Y && !nextInLML;}
    bool IsIntermediate(const int64_t Y) const {return ytop == Y && nextInLML;}


    TEdge *GetMaximaPair() const {
      if (!next->IsMaxima(ytop) || next->xtop != xtop)
        return prev;

      return next;
    }


    TEdge *GetNextInAEL(Direction dir) const {
      return dir == dLeftToRight ? nextInAEL : prevInAEL;
    }


    void SwapX() {
      // swap horizontal edges' top and bottom x's so they follow the natural
      // progression of the bounds - ie so their xbots will align with the
      // adjoining lower edge. [Helpful in the ProcessHorizontal() method.]
      xcurr = xtop;
      xtop = xbot;
      xbot = xcurr;
    }


    bool InsertsBefore(TEdge &e2) {
      if (e2.xcurr == xcurr) {
        if (e2.ytop > ytop) return e2.xtop < TopX(e2.ytop);
        return xtop > e2.TopX(ytop);
      }

      return e2.xcurr < xcurr;
    }


    void SwapSides(TEdge &edge2) {
      EdgeSide side = this->side;
      this->side = edge2.side;
      edge2.side = side;
    }


    void SwapPolyIndexes(TEdge &edge2) {
      int outIdx = this->outIdx;
      this->outIdx = edge2.outIdx;
      edge2.outIdx = outIdx;
    }


    int64_t TopX(const int64_t currentY) const {
      return currentY == ytop ? xtop : xbot + Round(dx * (currentY - ybot));
    }


    bool IntersectPoint(TEdge &edge2, IntPoint &ip,
                        bool UseFullInt64Range) const;
    bool SlopesEqual(const TEdge &e2, bool UseFullInt64Range) const;
  };


  typedef std::vector <TEdge *> EdgeList;
}
