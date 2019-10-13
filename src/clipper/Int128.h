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

#pragma once

#include <stdint.h>

// Int128 class (enables safe math on signed 64bit integers)
// eg Int128 val1((int64_t)9223372036854775807); // ie 2^63 -1
//    Int128 val2((int64_t)9223372036854775807);
//    Int128 val3 = val1 * val2;
//    val3.AsString => "85070591730234615847396907784232501249" (8.5e+37)


namespace ClipperLib {
  class Int128 {
  public:
    uint64_t lo;
    int64_t hi;

    Int128(int64_t lo = 0) : lo((uint64_t)lo), hi(lo < 0 ? -1 : 0) {}
    Int128(const Int128 &val) : lo(val.lo), hi(val.hi){}
    Int128(const int64_t &hi, const uint64_t &lo): lo(lo), hi(hi) {}


    int64_t operator=(const int64_t &val) {
      lo = (uint64_t)val;
      if (val < 0) hi = -1; else hi = 0;
      return val;
    }


    bool operator==(const Int128 &val) const {
      return hi == val.hi && lo == val.lo;
    }


    bool operator!=(const Int128 &val) const {return !(*this == val);}


    bool operator>(const Int128 &val) const {
      return hi == val.hi ? lo > val.lo : hi > val.hi;
    }


    bool operator<(const Int128 &val) const {
      return hi == val.hi ? lo < val.lo : hi < val.hi;
    }


    bool operator>=(const Int128 &val) const {return !(*this < val);}
    bool operator<=(const Int128 &val) const {return !(*this > val);}


    Int128 &operator+=(const Int128 &rhs) {
      hi += rhs.hi;
      lo += rhs.lo;
      if (lo < rhs.lo) hi++;
      return *this;
    }


    Int128 operator+(const Int128 &rhs) const {
      Int128 result(*this);
      result += rhs;
      return result;
    }


    Int128 &operator-=(const Int128 &rhs) {
      *this += -rhs;
      return *this;
    }


    Int128 operator-(const Int128 &rhs) const {
      Int128 result(*this);
      result -= rhs;
      return result;
    }


    Int128 operator-() const {
      return lo ? Int128(~hi, ~lo + 1) : Int128(-hi, 0);
    }


    Int128 operator/(const Int128 &rhs) const;
    double AsDouble() const;
  };


  Int128 Int128Mul(int64_t lhs, int64_t rhs);
}
