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
 * September 24-28, 2005, Long Beach, California, USA                          *
 * http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf             *
 *                                                                             *
 * This is a translation of the Delphi Clipper library and the naming style    *
 * used has retained a Delphi flavour.                                         *
 ******************************************************************************/

#include "OutRec.h"
#include "Int128.h"

using namespace ClipperLib;


double OutRec::Area(bool UseFullInt64Range) const {
  OutPt *op = pts;

  if (!op) return 0;

  if (UseFullInt64Range) {
    Int128 a(0);

    do {
      a += Int128Mul(op->pt.X + op->prev->pt.X, op->prev->pt.Y - op->pt.Y);
      op = op->next;
    } while (op != pts);

    return a.AsDouble() / 2;

  } else {
    double a = 0;
    do {
      a = a + (op->pt.X + op->prev->pt.X) * (op->prev->pt.Y - op->pt.Y);
      op = op->next;
    } while (op != pts);

    return a / 2;
  }
}


OutRec *OutRec::GetLowermostRec(OutRec *outRec2) {
  // work out which polygon fragment has the correct hole state
  if (!bottomPt) bottomPt = pts->GetBottomPt();
  if (!outRec2->bottomPt) outRec2->bottomPt = outRec2->pts->GetBottomPt();

  OutPt *outPt1 = bottomPt;
  OutPt *outPt2 = outRec2->bottomPt;

  if (outPt1->pt.Y > outPt2->pt.Y) return this;
  if (outPt1->pt.Y < outPt2->pt.Y) return outRec2;
  if (outPt1->pt.X < outPt2->pt.X) return this;
  if (outPt1->pt.X > outPt2->pt.X) return outRec2;
  if (outPt1->next == outPt1) return outRec2;
  if (outPt2->next == outPt2) return this;
  if (outPt1->FirstIsBottomPt(outPt2)) return this;
  return outRec2;
}


bool OutRec::Param1RightOfParam2(const OutRec *outRec2) const {
  const OutRec *outRec1 = this;

  do {
    outRec1 = outRec1->FirstLeft;
    if (outRec1 == outRec2) return true;
  } while (outRec1);

  return false;
}


void OutRec::UpdateOutPtIdxs() {
  OutPt *op = pts;

  do {
    op->idx = idx;
    op = op->prev;
  } while (op != pts);
}
