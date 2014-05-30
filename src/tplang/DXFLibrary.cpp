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

#include "DXFLibrary.h"

#include <openscam/dxf/DXFReader.h>
#include <openscam/dxf/DXFPoint.h>
#include <openscam/dxf/DXFLine.h>
#include <openscam/dxf/DXFArc.h>
#include <openscam/dxf/DXFPolyLine.h>
#include <openscam/dxf/DXFSpline.h>

#include <cbang/os/SystemUtilities.h>

using namespace tplang;
using namespace OpenSCAM;
using namespace cb;
using namespace std;


DXFLibrary::DXFLibrary(TPLContext &ctx) : js::Library(ctx), ctx(ctx) {
  entityTmpl.set("cut()", this, &DXFLibrary::cutCB);
}


void DXFLibrary::add(js::ObjectTemplate &tmpl) {
  tmpl.set("open(path)", this, &DXFLibrary::openCB);

  tmpl.set("POINT",    DXFEntity::DXF_POINT);
  tmpl.set("LINE",     DXFEntity::DXF_LINE);
  tmpl.set("ARC",      DXFEntity::DXF_ARC);
  tmpl.set("POLYLINE", DXFEntity::DXF_POLYLINE);
  tmpl.set("SPLINE",   DXFEntity::DXF_SPLINE);
}


js::Value DXFLibrary::openCB(const js::Arguments &args) {
  string path =
    SystemUtilities::absolute(ctx.currentPath(), args.getString("path"));

  DXFReader reader;
  reader.read(path);

  const DXFReader::layers_t &layers = reader.getLayers();
  js::Value v8Layers = layersTmpl.create();

  DXFReader::layers_t::const_iterator it;
  for (it = layers.begin(); it != layers.end(); it++) {
    const DXFReader::layer_t &layer = it->second;
    js::Value v8Layer = js::Value::createArray(layer.size());

    for (unsigned j = 0; j < layer.size(); j++) {
      const DXFEntity &entity = *layer[j];
      js::Value obj = entityTmpl.create();

      switch (entity.getType()) {
      case DXFEntity::DXF_POINT: {
        const DXFPoint &point = dynamic_cast<const DXFPoint &>(entity);
        obj.set("x", point.x());
        obj.set("y", point.y());
        obj.set("z", point.z());
        break;
      }

      case DXFEntity::DXF_LINE: {
        const DXFLine &line = dynamic_cast<const DXFLine &>(entity);

        js::Value start = entityTmpl.create();
        start.set("x", line.getStart().x());
        start.set("y", line.getStart().y());
        start.set("z", line.getStart().z());
        obj.set("start", start);

        js::Value end = entityTmpl.create();
        end.set("x", line.getEnd().x());
        end.set("y", line.getEnd().y());
        end.set("z", line.getEnd().z());
        obj.set("end", end);

        break;
      }

      case DXFEntity::DXF_ARC: {
        const DXFArc &arc = dynamic_cast<const DXFArc &>(entity);

        js::Value center = entityTmpl.create();
        center.set("x", arc.getCenter().x());
        center.set("y", arc.getCenter().y());
        center.set("z", arc.getCenter().z());
        obj.set("center", center);

        obj.set("radius", arc.getRadius());
        obj.set("startAngle", arc.getStartAngle());
        obj.set("endAngle", arc.getEndAngle());
        break;
      }

      case DXFEntity::DXF_POLYLINE: {
        const DXFPolyLine &polyLine = dynamic_cast<const DXFPolyLine &>(entity);
        const vector<Vector3D> &vertices = polyLine.getVertices();
        js::Value v8Vertices = js::Value::createArray(vertices.size());

        for (unsigned k = 0; k < vertices.size(); k++) {
          js::Value pt = entityTmpl.create();

          pt.set("x", vertices[k].x());
          pt.set("y", vertices[k].y());
          pt.set("z", vertices[k].z());
          pt.set("type", DXFEntity::DXF_POINT);

          v8Vertices.set(k, pt);
        }

        obj.set("vertices", v8Vertices);
        break;
      }

      case DXFEntity::DXF_SPLINE: {
        const DXFSpline &spline = dynamic_cast<const DXFSpline &>(entity);

        obj.set("degree", spline.getDegree());

        // Control points
        const vector<Vector3D> &ctrlPts = spline.getControlPoints();
        js::Value v8CtrlPts = js::Value::createArray(ctrlPts.size());

        for (unsigned k = 0; k < ctrlPts.size(); k++) {
          js::Value pt = entityTmpl.create();

          pt.set("x", ctrlPts[k].x());
          pt.set("y", ctrlPts[k].y());
          pt.set("z", ctrlPts[k].z());
          pt.set("type", DXFEntity::DXF_POINT);

          v8CtrlPts.set(k, pt);
        }
        obj.set("ctrlPts", v8CtrlPts);

        // Knots
        const vector<double> &knots = spline.getKnots();
        js::Value v8Knots = js::Value::createArray(knots.size());

        for (unsigned k = 0; k < knots.size(); k++)
          v8Knots.set(k, knots[k]);

        obj.set("knots", v8Knots);
        break;
      }

      default: THROWS("Invalid DXF entity type " << entity.getType());
      }

      obj.set("type", entity.getType());
      v8Layer.set(j, obj);
    }

    v8Layers.set(it->first, v8Layer);
  }

  return v8Layers;
}


js::Value DXFLibrary::cutCB(const js::Arguments &args) {
  return js::Value();
}
