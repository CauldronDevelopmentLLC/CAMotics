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


namespace ClipperLib {
  // By far the most widely used winding rules for polygon filling are
  // EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
  // Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
  // see http://glprogramming.com/red/chapter11.html
  enum PolyFillType {pftEvenOdd, pftNonZero, pftPositive, pftNegative};
  enum ClipType {ctIntersection, ctUnion, ctDifference, ctXor};
  enum PolyType {ptSubject, ptClip};
  enum JoinType {jtSquare, jtRound, jtMiter};
  enum EndType {etClosed, etButt, etSquare, etRound};
  enum EdgeSide {esNone = 0, esLeft = 1, esRight = 2};
  enum IntersectProtects {ipNone = 0, ipLeft = 1, ipRight = 2, ipBoth = 3};
  enum Direction {dRightToLeft, dLeftToRight};
}
