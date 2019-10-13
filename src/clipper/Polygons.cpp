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

#include "Polygons.h"

#include "Clipper.h"
#include "OffsetBuilder.h"
#include "Int128.h"

#include <algorithm>

using namespace ClipperLib;


void Polygons::Simplify(Polygons &out, PolyFillType fillType) const {
  Clipper c;

  c.ForceSimple(true);
  c.AddPolygons(*this, ptSubject);
  c.Execute(ctUnion, out, fillType, fillType);
}


void Polygons::Simplify(PolyFillType fillType) {
  Simplify(*this, fillType);
}


void Polygons::Offset(Polygons &out, double delta, JoinType jointype,
                      double limit, bool autoFix) const {
  if (!autoFix && this != &out) {
    OffsetBuilder(*this, out, true, delta, jointype, etClosed, limit);
    return;
  }

  Polygons inPolys = Polygons(*this);
  out.clear();

  // ChecksInput - fixes polygon orientation if necessary and removes
  // duplicate vertices. Can be set false when you're sure that polygon
  // orientation is correct and that there are no duplicate vertices.
  if (autoFix) {
    size_t polyCount = inPolys.size(), botPoly = 0;
    while (botPoly < polyCount && inPolys[botPoly].empty()) botPoly++;
    if (botPoly == polyCount) return;

    // botPt: used to find the lowermost (in inverted Y-axis) & leftmost point
    // This point (on m_p[botPoly]) must be on an outer polygon ring and if
    // its orientation is false (counterclockwise) then assume all polygons
    // need reversing
    IntPoint botPt = inPolys[botPoly][0];
    for (size_t i = botPoly; i < polyCount; i++) {
      if (inPolys[i].size() < 3) {inPolys[i].clear(); continue;}
      if (inPolys[i][0].UpdateBotPt(botPt)) botPoly = i;

      Polygon::iterator it = inPolys[i].begin() + 1;
      while (it != inPolys[i].end()) {
        if (it->Equal(*(it - 1))) it = inPolys[i].erase(it);
        else {
          if (it->UpdateBotPt(botPt)) botPoly = i;
          it++;
        }
      }
    }

    if (!inPolys[botPoly].Orientation()) inPolys.reverse();
  }

  OffsetBuilder(inPolys, out, true, delta, jointype, etClosed, limit);
}


void Polygons::Offset(double delta, JoinType joinType, double limit,
                      bool autoFix) {
  Offset(*this, delta, joinType, limit, autoFix);
}


void Polygons::OffsetPolyLines(Polygons &out, double delta, JoinType jointype,
                               EndType endtype, double limit,
                               bool autoFix) const {
  if (!autoFix && endtype != etClosed && this != &out) {
    OffsetBuilder(*this, out, false, delta, jointype, endtype, limit);
    return;
  }

  Polygons inLines = Polygons(*this);
  if (autoFix)
    for (size_t i = 0; i < inLines.size(); i++) {
      if (inLines[i].size() < 2) {inLines[i].clear(); continue;}
      Polygon::iterator it = inLines[i].begin() + 1;
      while (it != inLines[i].end()) {
        if (it->Equal(*(it - 1))) it = inLines[i].erase(it);
        else it++;
      }
    }

  if (endtype == etClosed) {
    size_t sz = inLines.size();
    inLines.resize(sz * 2);

    for (size_t i = 0; i < sz; i++) {
      inLines[sz + i] = inLines[i];
      inLines[sz + i].reverse();
    }

    OffsetBuilder(inLines, out, true, delta, jointype, endtype, limit);

  } else OffsetBuilder(inLines, out, false, delta, jointype, endtype, limit);
}


void Polygons::Clean(Polygons &out, double distance) const {
  out.resize(size());

  for (size_type i = 0; i < size(); i++)
    at(i).Clean(out[i], distance);
}


void Polygons::write(std::ostream &stream) const {
  for (auto it = begin(); it != end(); it++)
    stream << *it;

  stream << '\n';
}
