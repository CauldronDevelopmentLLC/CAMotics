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

#include "DXFModule.h"
#include "TPLContext.h"

#include <dxf/Reader.h>
#include <dxf/Point.h>
#include <dxf/Line.h>
#include <dxf/Arc.h>
#include <dxf/PolyLine.h>
#include <dxf/Spline.h>

#include <cbang/os/SystemUtilities.h>

using namespace tplang;
using namespace cb;
using namespace std;


DXFModule::DXFModule(TPLContext &ctx) : js::NativeModule("_dxf"), ctx(ctx) {}


void DXFModule::define(js::Sink &exports) {
  exports.insert("open(path)", this, &DXFModule::openCB);

  exports.insert("POINT",    DXF::Entity::DXF_POINT);
  exports.insert("LINE",     DXF::Entity::DXF_LINE);
  exports.insert("ARC",      DXF::Entity::DXF_ARC);
  exports.insert("POLYLINE", DXF::Entity::DXF_POLYLINE);
  exports.insert("SPLINE",   DXF::Entity::DXF_SPLINE);
}


void DXFModule::openCB(const js::Value &args, js::Sink &sink) {
  DXF::Reader reader;
  reader.read(ctx.relativePath(args.getString("path")));

  const DXF::Reader::layers_t &layers = reader.getLayers();
  sink.beginDict();

  DXF::Reader::layers_t::const_iterator it;
  for (it = layers.begin(); it != layers.end(); it++) {
    const DXF::Reader::layer_t &layer = it->second;
    sink.insertList(it->first);

    for (unsigned j = 0; j < layer.size(); j++) {
      const DXF::Entity &entity = *layer[j];
      sink.appendDict();

      switch (entity.getType()) {
      case DXF::Entity::DXF_POINT: {
        const DXF::Point &point = dynamic_cast<const DXF::Point &>(entity);
        sink.insert("x", point.x());
        sink.insert("y", point.y());
        sink.insert("z", point.z());
        break;
      }

      case DXF::Entity::DXF_LINE: {
        const DXF::Line &line = dynamic_cast<const DXF::Line &>(entity);

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

      case DXF::Entity::DXF_ARC: {
        const DXF::Arc &arc = dynamic_cast<const DXF::Arc &>(entity);

        sink.insertDict("center");
        sink.insert("x", arc.getCenter().x());
        sink.insert("y", arc.getCenter().y());
        sink.insert("z", arc.getCenter().z());
        sink.endDict();

        sink.insert("radius", arc.getRadius());
        sink.insert("startAngle", arc.getStartAngle());
        sink.insert("endAngle", arc.getEndAngle());
        sink.insertBoolean("clockwise", arc.getClockwise());
        break;
      }

      case DXF::Entity::DXF_POLYLINE: {
        const DXF::PolyLine &polyLine =
          dynamic_cast<const DXF::PolyLine &>(entity);
        const vector<Vector3D> &vertices = polyLine.getVertices();
        sink.insertList("vertices");

        for (unsigned k = 0; k < vertices.size(); k++) {
          sink.appendDict();
          sink.insert("x", vertices[k].x());
          sink.insert("y", vertices[k].y());
          sink.insert("z", vertices[k].z());
          sink.insert("type", DXF::Entity::DXF_POINT);
          sink.endDict();
        }

        sink.endList();
        break;
      }

      case DXF::Entity::DXF_SPLINE: {
        const DXF::Spline &spline = dynamic_cast<const DXF::Spline &>(entity);

        sink.insert("degree", spline.getDegree());

        // Control points
        const vector<Vector3D> &ctrlPts = spline.getControlPoints();
        sink.insertList("ctrlPts");

        for (unsigned k = 0; k < ctrlPts.size(); k++) {
          sink.appendDict();
          sink.insert("x", ctrlPts[k].x());
          sink.insert("y", ctrlPts[k].y());
          sink.insert("z", ctrlPts[k].z());
          sink.insert("type", DXF::Entity::DXF_POINT);
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

      default: THROW("Invalid DXF entity type " << entity.getType());
      }

      sink.insert("type", entity.getType());
      sink.endDict();
    }

    sink.endList();
  }

  sink.endDict();
}
