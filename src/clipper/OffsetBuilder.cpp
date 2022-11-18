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

#include "OffsetBuilder.h"
#include "Util.h"
#include "Clipper.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ClipperLib;


namespace {
  Polygon BuildArc(const IntPoint &pt, const double a1, const double a2,
                   const double r, double limit) {
    // see notes in clipper.pas regarding steps
    double arcFrac = std::fabs(a2 - a1) / (2 * M_PI);
    int steps = (int)(arcFrac * M_PI / std::acos(1 - limit / std::fabs(r)));
    if (steps < 2) steps = 2;
    else if (steps > (int)(222.0 * arcFrac)) steps = (int)(222.0 * arcFrac);

    double x = std::cos(a1);
    double y = std::sin(a1);
    double c = std::cos((a2 - a1) / steps);
    double s = std::sin((a2 - a1) / steps);
    Polygon result(steps + 1);

    for (int i = 0; i <= steps; i++) {
      result[i].X = pt.X + Round(x * r);
      result[i].Y = pt.Y + Round(y * r);
      double x2 = x;
      x = x * c - s * y;  // cross product
      y = x2 * s + y * c; // dot product
    }

    return result;
  }
}


OffsetBuilder::OffsetBuilder(const Polygons &in_polys, Polygons &out_polys,
                             bool isPolygon, double delta, JoinType jointype,
                             EndType endtype, double limit) : m_p(in_polys) {
  // precondition: &out_polys != &in_polys

  if (CLIPPER_NEAR_ZERO(delta)) {out_polys = in_polys; return;}
  m_rmin = 0.5;
  m_delta = delta;

  if (jointype == jtMiter) {
    if (limit > 2) m_rmin = 2.0 / (limit * limit);
    limit = 0.25; // just in case endtype == etRound

  } else {
    if (limit <= 0) limit = 0.25;
    else if (limit > std::fabs(delta)) limit = std::fabs(delta);
  }

  out_polys.clear();
  out_polys.resize(m_p.size());
  for (m_i = 0; m_i < m_p.size(); m_i++) {
    size_t len = m_p[m_i].size();

    if (len == 0 || (len < 3 && delta <= 0)) continue;
    else if (len == 1) {
      out_polys[m_i] = BuildArc(m_p[m_i][0], 0, 2 * M_PI, delta, limit);
      continue;
    }

    bool forceClose = m_p[m_i][0].Equal(m_p[m_i][len - 1]);
    if (forceClose) len--;

    // build normals
    normals.clear();
    normals.resize(len);
    for (m_j = 0; m_j < len - 1; m_j++)
      normals[m_j] = m_p[m_i][m_j].GetUnitNormal(m_p[m_i][m_j + 1]);
    if (isPolygon || forceClose)
      normals[len - 1] = m_p[m_i][len - 1].GetUnitNormal(m_p[m_i][0]);
    else normals[len - 1] = normals[len - 2]; // is open polyline

    m_curr_poly = &out_polys[m_i];
    m_curr_poly->reserve(len);

    if (isPolygon || forceClose) {
      m_k = len - 1;
      for (m_j = 0; m_j < len; ++m_j)
        OffsetPoint(jointype, limit);

      if (!isPolygon) {
        size_t j = out_polys.size();
        out_polys.resize(j + 1);
        m_curr_poly = &out_polys[j];
        m_curr_poly->reserve(len);
        m_delta = -m_delta;

        m_k = len - 1;
        for (m_j = 0; m_j < len; ++m_j)
          OffsetPoint(jointype, limit);
        m_delta = -m_delta;
        m_curr_poly->reverse();
      }

    } else { // is open polyline
      // offset the polyline going forward
      m_k = 0;
      for (m_j = 1; m_j < len - 1; ++m_j)
        OffsetPoint(jointype, limit);

      // handle the end (butt, round or square)
      IntPoint pt1;
      if (endtype == etButt) {
        m_j = len - 1;
        pt1 = IntPoint(Round(m_p[m_i][m_j].X + normals[m_j].X * m_delta),
                       Round(m_p[m_i][m_j].Y + normals[m_j].Y * m_delta));
        AddPoint(pt1);
        pt1 = IntPoint(Round(m_p[m_i][m_j].X - normals[m_j].X * m_delta),
                       Round(m_p[m_i][m_j].Y - normals[m_j].Y * m_delta));
        AddPoint(pt1);

      } else {
        m_j = len - 1;
        m_k = len - 2;
        normals[m_j].X = -normals[m_j].X;
        normals[m_j].Y = -normals[m_j].Y;
        if (endtype == etSquare) DoSquare();
        else DoRound(limit);
      }

      // re-build Normals
      for (int j = len - 1; j > 0; j--) {
        normals[j].X = -normals[j - 1].X;
        normals[j].Y = -normals[j - 1].Y;
      }

      normals[0].X = -normals[1].X;
      normals[0].Y = -normals[1].Y;

      // offset the polyline going backward
      m_k = len - 1;
      for (m_j = m_k - 1; m_j > 0; m_j--)
        OffsetPoint(jointype, limit);

      // finally handle the start (butt, round or square)
      if (endtype == etButt) {
        pt1 = IntPoint(Round(m_p[m_i][0].X - normals[0].X * m_delta),
                       Round(m_p[m_i][0].Y - normals[0].Y * m_delta));
        AddPoint(pt1);
        pt1 = IntPoint(Round(m_p[m_i][0].X + normals[0].X * m_delta),
                       Round(m_p[m_i][0].Y + normals[0].Y * m_delta));
        AddPoint(pt1);

      } else {
        m_k = 1;
        if (endtype == etSquare) DoSquare();
        else DoRound(limit);
      }
    }
  }

  // and clean up untidy corners using Clipper
  Clipper clpr;
  clpr.AddPolygons(out_polys, ptSubject);

  if (delta > 0) {
    if (!clpr.Execute(ctUnion, out_polys, pftPositive, pftPositive))
      out_polys.clear();

  } else {
    Bounds r = clpr.GetBounds();
    Polygon outer(4);

    outer[0] = IntPoint(r.left - 10, r.bottom + 10);
    outer[1] = IntPoint(r.right + 10, r.bottom + 10);
    outer[2] = IntPoint(r.right + 10, r.top - 10);
    outer[3] = IntPoint(r.left - 10, r.top - 10);

    clpr.AddPolygon(outer, ptSubject);
    clpr.ReverseSolution(true);

    if (clpr.Execute(ctUnion, out_polys, pftNegative, pftNegative))
      out_polys.erase(out_polys.begin());
    else out_polys.clear();
  }
}


void OffsetBuilder::OffsetPoint(JoinType jointype, double limit) {
  switch (jointype) {
  case jtMiter: {
    m_r = 1 + (normals[m_j].X*normals[m_k].X +
               normals[m_j].Y*normals[m_k].Y);
    if (m_r >= m_rmin) DoMiter(); else DoSquare();
    break;
  }

  case jtSquare: DoSquare(); break;
  case jtRound: DoRound(limit); break;
  }

  m_k = m_j;
}


void OffsetBuilder::AddPoint(const IntPoint &pt) {
  if (m_curr_poly->size() == m_curr_poly->capacity())
    m_curr_poly->reserve(m_curr_poly->capacity() + buffLength);
  m_curr_poly->push_back(pt);
}


void OffsetBuilder::DoSquare() {
  IntPoint pt1 =
    IntPoint(Round(m_p[m_i][m_j].X + normals[m_k].X * m_delta),
             Round(m_p[m_i][m_j].Y + normals[m_k].Y * m_delta));
  IntPoint pt2 =
    IntPoint(Round(m_p[m_i][m_j].X + normals[m_j].X * m_delta),
             Round(m_p[m_i][m_j].Y + normals[m_j].Y * m_delta));

  if ((normals[m_k].X * normals[m_j].Y - normals[m_j].X * normals[m_k].Y) *
      m_delta >= 0) {
    double a1 = std::atan2(normals[m_k].Y, normals[m_k].X);
    double a2 = std::atan2(-normals[m_j].Y, -normals[m_j].X);
    a1 = std::fabs(a2 - a1);

    if (a1 > M_PI) a1 = M_PI * 2 - a1;
    double dx = std::tan((M_PI - a1) / 4) * std::fabs(m_delta);

    pt1 = IntPoint((int64_t)(pt1.X - normals[m_k].Y * dx),
                   (int64_t)(pt1.Y + normals[m_k].X * dx));
    AddPoint(pt1);
    pt2 = IntPoint((int64_t)(pt2.X + normals[m_j].Y * dx),
                   (int64_t)(pt2.Y - normals[m_j].X * dx));
    AddPoint(pt2);

  } else {
    AddPoint(pt1);
    AddPoint(m_p[m_i][m_j]);
    AddPoint(pt2);
  }
}


void OffsetBuilder::DoMiter() {
  if ((normals[m_k].X * normals[m_j].Y - normals[m_j].X * normals[m_k].Y) *
      m_delta >= 0) {
    double q = m_delta / m_r;
    AddPoint(IntPoint(Round(m_p[m_i][m_j].X +
                            (normals[m_k].X + normals[m_j].X) * q),
                      Round(m_p[m_i][m_j].Y +
                            (normals[m_k].Y + normals[m_j].Y) * q)));
  } else {
    IntPoint pt1 =
      IntPoint(Round(m_p[m_i][m_j].X + normals[m_k].X * m_delta),
               Round(m_p[m_i][m_j].Y + normals[m_k].Y * m_delta));
    IntPoint pt2 =
      IntPoint(Round(m_p[m_i][m_j].X + normals[m_j].X * m_delta),
               Round(m_p[m_i][m_j].Y + normals[m_j].Y * m_delta));

    AddPoint(pt1);
    AddPoint(m_p[m_i][m_j]);
    AddPoint(pt2);
  }
}


void OffsetBuilder::DoRound(double limit) {
  IntPoint pt1 =
    IntPoint(Round(m_p[m_i][m_j].X + normals[m_k].X * m_delta),
             Round(m_p[m_i][m_j].Y + normals[m_k].Y * m_delta));
  IntPoint pt2 =
    IntPoint(Round(m_p[m_i][m_j].X + normals[m_j].X * m_delta),
             Round(m_p[m_i][m_j].Y + normals[m_j].Y * m_delta));

  AddPoint(pt1);

  // round off reflex angles (ie > 180 deg) unless almost flat (ie < ~10deg)
  if ((normals[m_k].X * normals[m_j].Y - normals[m_j].X * normals[m_k].Y) *
      m_delta >= 0) {
    if (normals[m_j].X * normals[m_k].X + normals[m_j].Y * normals[m_k].Y <
        0.985) {
      double a1 = std::atan2(normals[m_k].Y, normals[m_k].X);
      double a2 = std::atan2(normals[m_j].Y, normals[m_j].X);

      if (m_delta > 0 && a2 < a1) a2 += M_PI * 2;
      else if (m_delta < 0 && a2 > a1) a2 -= M_PI * 2;

      Polygon arc = BuildArc(m_p[m_i][m_j], a1, a2, m_delta, limit);
      for (Polygon::size_type m = 0; m < arc.size(); m++)
        AddPoint(arc[m]);
    }

  } else AddPoint(m_p[m_i][m_j]);

  AddPoint(pt2);
}
