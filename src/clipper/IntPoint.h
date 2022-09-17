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
#include <ostream>


namespace ClipperLib {
  struct DoublePoint {
    double X;
    double Y;

    DoublePoint(double x = 0, double y = 0) : X(x), Y(y) {}
  };


  struct IntPoint {
  public:
    int64_t X;
    int64_t Y;

    IntPoint(int64_t x = 0, int64_t y = 0) : X(x), Y(y) {}

    double GetDx(const IntPoint &pt2) const;
    void RangeTest(int64_t &maxrange) const;
    bool PointsAreClose(const IntPoint &pt2, double distSqrd) const;
    bool UpdateBotPt(IntPoint &botPt) const;

    bool Equal(const IntPoint &pt2) const;
    void Swap(IntPoint &pt2);
    bool OnLineSegment(const IntPoint linePt1, const IntPoint linePt2,
                       bool UseFullInt64Range) const;
    DoublePoint GetUnitNormal(const IntPoint &pt2) const;
    double DistanceSqrd(const IntPoint &pt2) const;
    DoublePoint ClosestPointOnLine(const IntPoint &linePt1,
                                   const IntPoint &linePt2) const;
    bool IsBetween(const IntPoint pt1, const IntPoint pt2) const;

    void write(std::ostream &stream) const;
  };


  bool GetOverlapSegment(IntPoint pt1a, IntPoint pt1b, IntPoint pt2a,
                         IntPoint pt2b, IntPoint &pt1, IntPoint &pt2);
  bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3,
                   bool UseFullInt64Range);
  bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3,
                   const IntPoint pt4, bool UseFullInt64Range);
  bool SlopesNearColinear(const IntPoint &pt1, const IntPoint &pt2,
                          const IntPoint &pt3, double distSqrd);

  static inline
  std::ostream &operator<<(std::ostream &stream, const IntPoint &p) {
    p.write(stream);
    return stream;
  }
}
