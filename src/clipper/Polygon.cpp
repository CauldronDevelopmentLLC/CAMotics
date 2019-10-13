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

#include "Polygon.h"

#include "Clipper.h"
#include "Int128.h"

#include <algorithm>

using namespace ClipperLib;


bool Polygon::FullRangeNeeded() const {
  bool result = false;

  for (Polygon::size_type i = 0; i <  size(); i++) {
    if (Abs(at(i).X) > CLIPPER_HI || Abs(at(i).Y) > CLIPPER_HI)
      throw "Coordinate exceeds range bounds.";

    if (Abs(at(i).X) > CLIPPER_LO || Abs(at(i).Y) > CLIPPER_LO)
      result = true;
  }

  return result;
}


bool Polygon::Orientation() const {return 0 <= Area();}


double Polygon::Area() const {
  int highI = (int)size() - 1;
  if (highI < 2) return 0;

  if (FullRangeNeeded()) {
    Int128 a = Int128Mul(at(highI).X + at(0).X, at(0).Y - at(highI).Y);

    for (int i = 1; i <= highI; i++)
      a += Int128Mul(at(i - 1).X + at(i).X, at(i).Y - at(i - 1).Y);

    return a.AsDouble() / 2;

  } else {
    double a = ((double)at(highI).X + at(0).X) *
      ((double)at(0).Y - at(highI).Y);

    for (int i = 1; i <= highI; i++)
      a += ((double)at(i - 1).X + at(i).X) * ((double)at(i).Y - at(i - 1).Y);

    return a / 2;
  }
}


void Polygon::Simplify(Polygons &out, PolyFillType fillType) const {
  Clipper c;

  c.ForceSimple(true);
  c.AddPolygon(*this, ptSubject);
  c.Execute(ctUnion, out, fillType, fillType);
}


void Polygon::Clean(Polygon &out, double distance) const {
  // distance = proximity in units/pixels below which vertices
  // will be stripped. Default ~= sqrt(2).
  int highI = size() - 1;
  double distSqrd = distance * distance;

  while (highI > 0 && at(highI).PointsAreClose(at(0), distSqrd))
    highI--;

  if (highI < 2) {out.clear(); return;}

  if (this != &out) out.resize(highI + 1);

  IntPoint pt = at(highI);
  int i = 0, k = 0;

  while (true) {
    while (i < highI && pt.PointsAreClose(at(i + 1), distSqrd)) i += 2;
    int i2 = i;
    while (i < highI &&
           (at(i).PointsAreClose(at(i + 1), distSqrd) ||
            SlopesNearColinear(pt, at(i), at(i + 1), distSqrd))) i++;
    if (i >= highI) break;
    if (i != i2) continue;

    pt = at(i++);
    out[k++] = pt;
  }

  if (i <= highI) out[k++] = at(i);
  if (k > 2 && SlopesNearColinear(out[k - 2], out[k - 1], out[0], distSqrd))
    k--;

  if (k < 3) out.clear();
  else if (k <= highI) out.resize(k);
}


void Polygon::write(std::ostream &stream) const {
  for (auto it = begin(); it != end(); it++)
    stream << *it;

  stream << '\n';
}
