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

#include <stdint.h>


#define CLIPPER_HORIZONTAL (-1.0E+40)
#define CLIPPER_TOLERANCE (1.0e-20)
#define CLIPPER_NEAR_ZERO(val) \
  (((val) > -CLIPPER_TOLERANCE) && ((val) < CLIPPER_TOLERANCE))
#define CLIPPER_NEAR_EQUAL(a, b) CLIPPER_NEAR_ZERO((a) - (b))
#define CLIPPER_LO ((int64_t)0x3FFFFFFF)
#define CLIPPER_HI ((int64_t)0x3FFFFFFFFFFFFFFFLL)


namespace ClipperLib {
  inline int64_t Abs(int64_t val) {return val < 0 ? -val : val;}


  inline int64_t Round(double val) {
    return val < 0 ? static_cast<int64_t>(val - 0.5) :
      static_cast<int64_t>(val + 0.5);
  }
}
