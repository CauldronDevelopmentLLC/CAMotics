/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "DXFModule.h"
#include "TPLContext.h"

#include <camotics/dxf/DXFReader.h>
#include <camotics/dxf/DXFPoint.h>
#include <camotics/dxf/DXFLine.h>
#include <camotics/dxf/DXFArc.h>
#include <camotics/dxf/DXFPolyLine.h>
#include <camotics/dxf/DXFSpline.h>

#include <cbang/os/SystemUtilities.h>

using namespace tplang;
using namespace CAMotics;
using namespace cb;
using namespace std;


DXFModule::DXFModule(TPLContext &ctx) : js::NativeModule("_dxf"), ctx(ctx) {}


void DXFModule::define(js::Sink &exports) {
  exports.insert("open(path)", this, &DXFModule::openCB);

  exports.insert("POINT",    DXFEntity::DXF_POINT);
  exports.insert("LINE",     DXFEntity::DXF_LINE);
  exports.insert("ARC",      DXFEntity::DXF_ARC);
  exports.insert("POLYLINE", DXFEntity::DXF_POLYLINE);
  exports.insert("SPLINE",   DXFEntity::DXF_SPLINE);
}


void DXFModule::openCB(const js::Value &args, js::Sink &sink) {
  DXFReader reader;
  reader.read(ctx.relativePath(args.getString("path")));

  const DXFReader::layers_t &layers = reader.getLayers();
  sink.beginDict();

  DXFReader::layers_t::const_iterator it;
  for (it = layers.begin(); it != layers.end(); it++) {
    const DXFReader::layer_t &layer = it->second;
    sink.insertList(it->first);

    for (unsigned j = 0; j < layer.size(); j++) {
      const DXFEntity &entity = *layer[j];
      sink.appendDict();

      switch (entity.getType()) {
      case DXFEntity::DXF_POINT: {
        const DXFPoint &point = dynamic_cast<const DXFPoint &>(entity);
        sink.insert("x", point.x());
        sink.insert("y", point.y());
        sink.insert("z", point.z());
        break;
      }

      case DXFEntity::DXF_LINE: {
        const DXFLine &line = dynamic_cast<const DXFLine &>(entity);

        sink.insertDict("start");
        sink.insert("x", line.getStart().x());
        sink.insert("y", line.getStart().y());
        sink.insert("z", line.getStart().z());
        sink.endDict();

        sink.insertDict("end");
        sink.insert("x", line.getEnd().x());
        sink.insert("y", line.getEnd().y());
        sink.insert("z", line.getEnd().z());
        sink.endDict();
        break;
      }

      case DXFEntity::DXF_ARC: {
        const DXFArc &arc = dynamic_cast<const DXFArc &>(entity);

        sink.insertDict("center");
        sink.insert("x", arc.getCenter().x());
        sink.insert("y", arc.getCenter().y());
        sink.insert("z", arc.getCenter().z());
        sink.endDict();

        sink.insert("radius", arc.getRadius());
        sink.insert("startAngle", arc.getStartAngle());
        sink.insert("endAngle", arc.getEndAngle());
        sink.insert("clockwise", arc.getClockwise());
        break;
      }

      case DXFEntity::DXF_POLYLINE: {
        const DXFPolyLine &polyLine = dynamic_cast<const DXFPolyLine &>(entity);
        const vector<Vector3D> &vertices = polyLine.getVertices();
        sink.insertList("vertices");

        for (unsigned k = 0; k < vertices.size(); k++) {
          sink.appendDict();
          sink.insert("x", vertices[k].x());
          sink.insert("y", vertices[k].y());
          sink.insert("z", vertices[k].z());
          sink.insert("type", DXFEntity::DXF_POINT);
          sink.endDict();
        }

        sink.endList();
        break;
      }

      case DXFEntity::DXF_SPLINE: {
        const DXFSpline &spline = dynamic_cast<const DXFSpline &>(entity);

        sink.insert("degree", spline.getDegree());

        // Control points
        const vector<Vector3D> &ctrlPts = spline.getControlPoints();
        sink.insertList("ctrlPts");

        for (unsigned k = 0; k < ctrlPts.size(); k++) {
          sink.appendDict();
          sink.insert("x", ctrlPts[k].x());
          sink.insert("y", ctrlPts[k].y());
          sink.insert("z", ctrlPts[k].z());
          sink.insert("type", DXFEntity::DXF_POINT);
          sink.endDict();
        }

        sink.endList();

        // Knots
        const vector<double> &knots = spline.getKnots();
        sink.insertList("knots");

        for (unsigned k = 0; k < knots.size(); k++)
          sink.append(knots[k]);

        sink.endList();
        break;
      }

      default: THROWS("Invalid DXF entity type " << entity.getType());
      }

      sink.insert("type", entity.getType());
      sink.endDict();
    }

    sink.endList();
  }

  sink.endDict();
}
