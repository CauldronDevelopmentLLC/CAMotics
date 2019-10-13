/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "ClipperModule.h"

#include <cbang/json/JSON.h>

#include <clipper/Clipper.h>

using namespace cb;
using namespace ClipperLib;
using namespace tplang;


void ClipperModule::define(js::Sink &exports) {
  exports.insert("offset(polys, delta, join, limit=1000, autoFix=true, "
                 "scale=1000000)", this, &ClipperModule::offsetCB);

  exports.insert("JOIN_SQUARE", jtSquare);
  exports.insert("JOIN_ROUND", jtRound);
  exports.insert("JOIN_MITER", jtMiter);
}


void ClipperModule::offsetCB(const js::Value &args, js::Sink &sink) {
  int scale = args.getInteger("scale");

  // Convert JavaScript polys to Clipper polys
  Polygons polys;
  SmartPointer<js::Value> jsPolys = args.get("polys");
  bool dict = false;

  for (unsigned i = 0; i < jsPolys->length(); i++) {
    polys.push_back(Polygon());
    Polygon &poly = polys.back();
    SmartPointer<js::Value> jsPoly = jsPolys->get(i);

    for (unsigned j = 0; j < jsPoly->length(); j++) {
      SmartPointer<js::Value> jsPoint = jsPoly->get(j);

      if (jsPoint->has("x") && jsPoint->has("y")) {
        poly.push_back(IntPoint(jsPoint->getNumber("x") * scale,
                                jsPoint->getNumber("y") * scale));
        dict = true;

      } else if (jsPoint->length() == 2)
        poly.push_back(IntPoint(jsPoint->getNumber(0) * scale,
                                jsPoint->getNumber(1) * scale));

      else THROW("Expected 2D point");
    }
  }

  double delta = args.getNumber("delta") * scale;
  JoinType join =
    args.has("join") ? (JoinType)args.getInteger("join") : jtRound;
  double limit = args.getNumber("limit") * scale;
  bool autoFix = args.getBoolean("autoFix");

  polys.Offset(delta, join, limit, autoFix);

  // Convert Clipper result back to JavaScript
  sink.beginList();
  for (unsigned i = 0; i < polys.size(); i++) {
    Polygon &poly = polys[i];
    sink.appendList();

    for (unsigned j = 0; j < poly.size(); j++) {
      IntPoint &point = poly[j];

      if (dict) {
        sink.appendDict();
        sink.insert("x", (double)point.X / scale);
        sink.insert("y", (double)point.Y / scale);
        sink.endDict();

      } else {
        sink.appendList();
        sink.append((double)point.X / scale);
        sink.append((double)point.Y / scale);
        sink.endList();
      }
    }

    sink.endList();
  }

  sink.endList();
}
