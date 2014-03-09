/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

#include "ClipperLibrary.h"

#include <clipper/clipper.hpp>

using namespace cb;
using namespace ClipperLib;
using namespace tplang;


void ClipperLibrary::add(js::ObjectTemplate &tmpl) {
  tmpl.set("offset(polys, delta, join, limit=1000, autoFix=true, "
           "scale=1000000)", this, &ClipperLibrary::offsetCB);

  tmpl.set("JOIN_SQUARE", jtSquare);
  tmpl.set("JOIN_ROUND", jtRound);
  tmpl.set("JOIN_MITER", jtMiter);
}


js::Value ClipperLibrary::offsetCB(const js::Arguments &args) {
  uint32_t scale = args.getUint32("scale");

  // Convert JavaScript polys to Clipper polys
  Polygons polys;
  js::Value jsPolys = args.get("polys");

  for (int i = 0; i < jsPolys.length(); i++) {
    polys.push_back(Polygon());
    Polygon &poly = polys.back();
    js::Value jsPoly = jsPolys.get(i);

    for (int j = 0; j < jsPoly.length(); j++) {
      js::Value jsPoint = jsPoly.get(j);

      if (jsPoint.length() != 2) THROWS("Expected 2D point");
      poly.push_back(IntPoint(jsPoint.get(0).toNumber() * scale,
                              jsPoint.get(1).toNumber() * scale));
    }
  }

  double delta = args.getNumber("delta") * scale;
  JoinType join = args.has("join") ? (JoinType)args.getUint32("join") : jtRound;
  double limit = args.getNumber("limit") * scale;
  bool autoFix = args.getBoolean("autoFix");

  OffsetPolygons(polys, polys, delta, join, limit, autoFix);

  // Convert Clipper result back to JavaScript
  jsPolys = js::Value::createArray(polys.size());
  for (unsigned i = 0; i < polys.size(); i++) {
    Polygon &poly = polys[i];
    js::Value jsPoly = js::Value::createArray(poly.size());

    for (unsigned j = 0; j < poly.size(); j++) {
      IntPoint &point = poly[j];
      js::Value jsPoint = js::Value::createArray(2);

      jsPoint.set(0, (double)point.X / scale);
      jsPoint.set(1, (double)point.Y / scale);
      jsPoly.set(j, jsPoint);
    }

    jsPolys.set(i, jsPoly);
  }

  return jsPolys;
}
