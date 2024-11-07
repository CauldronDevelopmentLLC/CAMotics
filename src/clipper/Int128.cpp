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

#include "Int128.h"

using namespace ClipperLib;


Int128 Int128::operator/(const Int128 &rhs) const {
  if (!rhs.lo && !rhs.hi) throw "Int128 operator/: divide by zero";

  bool negate = (rhs.hi < 0) != (hi < 0);
  Int128 dividend = *this;
  Int128 divisor = rhs;

  if (dividend.hi < 0) dividend = -dividend;
  if (divisor.hi < 0) divisor = -divisor;

  if (divisor < dividend) {
    Int128 result = Int128(0);
    Int128 cntr = Int128(1);

    while (divisor.hi >= 0 && !(divisor > dividend)) {
      divisor.hi <<= 1;
      if ((int64_t)divisor.lo < 0) divisor.hi++;
      divisor.lo <<= 1;

      cntr.hi <<= 1;
      if ((int64_t)cntr.lo < 0) cntr.hi++;
      cntr.lo <<= 1;
    }

    divisor.lo >>= 1;
    if ((divisor.hi & 1) == 1) divisor.lo |= 0x8000000000000000LL;
    divisor.hi = (uint64_t)divisor.hi >> 1;

    cntr.lo >>= 1;
    if ((cntr.hi & 1) == 1) cntr.lo |= 0x8000000000000000LL;
    cntr.hi >>= 1;

    while (cntr.hi || cntr.lo) {
      if (divisor <= dividend) {
        dividend -= divisor;
        result.hi |= cntr.hi;
        result.lo |= cntr.lo;
      }

      divisor.lo >>= 1;
      if ((divisor.hi & 1) == 1) divisor.lo |= 0x8000000000000000LL;
      divisor.hi >>= 1;

      cntr.lo >>= 1;
      if ((cntr.hi & 1) == 1) cntr.lo |= 0x8000000000000000LL;
      cntr.hi >>= 1;
    }

    if (negate) result = -result;
    return result;

  } else if (rhs.hi == this->hi && rhs.lo == this->lo) return Int128(1);
  else return Int128(0);
}


double Int128::AsDouble() const {
  const double shift64 = 18446744073709551616.0; // 2^64

  if (hi < 0) {
    if (lo == 0) return (double)hi * shift64;
    else return -(double)(~lo + ~hi * shift64);
  } else return (double)(lo + hi * shift64);
}


namespace ClipperLib {
  Int128 Int128Mul(int64_t lhs, int64_t rhs) {
    bool negate = (lhs < 0) != (rhs < 0);

    if (lhs < 0) lhs = -lhs;
    uint64_t int1Hi = uint64_t(lhs) >> 32;
    uint64_t int1Lo = uint64_t(lhs & 0xFFFFFFFF);

    if (rhs < 0) rhs = -rhs;
    uint64_t int2Hi = uint64_t(rhs) >> 32;
    uint64_t int2Lo = uint64_t(rhs & 0xFFFFFFFF);

    uint64_t a = int1Hi * int2Hi;
    uint64_t b = int1Lo * int2Lo;
    uint64_t c = int1Hi * int2Lo + int1Lo * int2Hi;

    Int128 tmp;
    tmp.hi = int64_t(a + (c >> 32));
    tmp.lo = int64_t(c << 32);
    tmp.lo += int64_t(b);

    if (tmp.lo < b) tmp.hi++;
    if (negate) tmp = -tmp;

    return tmp;
  }
}
