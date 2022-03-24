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

#include "Reader.h"
#include "Point.h"
#include "Arc.h"
#include "Line.h"
#include "PolyLine.h"
#include "Spline.h"

#include <dxflib/dl_dxf.h>

#include <cbang/log/Logger.h>

using namespace cb;
using namespace DXF;


void Reader::read(const InputSource &source) {
  SmartPointer<DL_Dxf> dxf = new DL_Dxf;

  if (!dxf->in(source.getStream(), this))
    THROW("Failed to read '" << source << "' as DXF");
}


void Reader::addEntity(const SmartPointer<Entity> &entity) {
  if (inBlock) return; // TODO handle blocks

  auto layer = attributes.getLayer();
  layers_t::iterator it = layers.find(layer);
  if (it == layers.end())
    it = layers.insert(layers_t::value_type(layer, layer_t())).first;

  it->second.push_back(entity);
}


void Reader::addLayer(const DL_LayerData &data) {
  LOG_DEBUG(3, "Adding DXF layer " << data.name);
  layers[data.name] = layer_t();
}


void Reader::addPoint(const DL_PointData &point) {
  addEntity(new Point(cb::Vector3D(point.x, point.y, point.z)));
}


void Reader::addLine(const DL_LineData &line) {
  addEntity(new Line(cb::Vector3D(line.x1, line.y1, line.z1),
                        cb::Vector3D(line.x2, line.y2, line.z2)));
}


void Reader::addArc(const DL_ArcData &arc) {
  addEntity(new Arc(cb::Vector3D(arc.cx, arc.cy, arc.cz), arc.radius,
                    arc.angle1, arc.angle2,
                    0 < getExtrusion()->getDirection()[2]));
}


void Reader::addCircle(const DL_CircleData &circle) {
  addEntity(new Arc(cb::Vector3D(circle.cx, circle.cy, circle.cz),
                    circle.radius, 0, 360, true));
}


void Reader::addPolyline(const DL_PolylineData &polyline) {
  if (!entity.isNull()) THROW("DXF Already in DXF entity");
  addEntity(entity = new PolyLine);
}


void Reader::addVertex(const DL_VertexData &vertex) {
  if (vertex.bulge) LOG_WARNING("Cannot handle vertex with bulge");
  entity->addVertex(cb::Vector3D(vertex.x, vertex.y, vertex.z));
}


void Reader::addSpline(const DL_SplineData &spline) {
  if (!entity.isNull()) THROW("DXF Already in DXF entity");
  addEntity(entity = new Spline(spline.degree));
}


void Reader::addControlPoint(const DL_ControlPointData &ctrlPt) {
  entity->addVertex(cb::Vector3D(ctrlPt.x, ctrlPt.y, ctrlPt.z));
}


void Reader::addKnot(const DL_KnotData &knot) {
  entity->addKnot(knot.k);
}


void Reader::addEllipse(const DL_EllipseData &ellipse) {
  if (warnEllipse) LOG_WARNING("DXF Ellipse not supported");
  warnEllipse = false;
}


void Reader::add3dFace(const DL_3dFaceData &face) {
  if (warn3DFace) LOG_WARNING("DXF 3D Face not supported");
  warn3DFace = false;
}


void Reader::addSolid(const DL_SolidData &solid) {
  if (warnSolid) LOG_WARNING("DXF Solid not supported");
  warnSolid = false;
}
