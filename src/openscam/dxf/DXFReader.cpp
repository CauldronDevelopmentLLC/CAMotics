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

#include "DXFReader.h"
#include "DXFPoint.h"
#include "DXFArc.h"
#include "DXFLine.h"
#include "DXFPolyLine.h"
#include "DXFSpline.h"

#include "dxflib/dl_dxf.h"

#include <cbang/log/Logger.h>

using namespace cb;
using namespace OpenSCAM;


void DXFReader::read(const InputSource &source) {
  if (!DL_Dxf().in(source.getStream(), this))
    THROWS("Failed to read '" << source << "' as DXF");
}


void DXFReader::addEntity(const SmartPointer<DXFEntity> &entity) {
  layers_t::iterator it = layers.find(attributes.getLayer());
  if (it == layers.end())
    THROWS("DXF Undefined layer '" << attributes.getLayer() << "'");

  it->second.push_back(entity);
}


void DXFReader::addLayer(const DL_LayerData &data) {
  layers[data.name] = layer_t();
}


void DXFReader::addPoint(const DL_PointData &point) {
  addEntity(new DXFPoint(Vector3D(point.x, point.y, point.z)));
}


void DXFReader::addLine(const DL_LineData &line) {
  addEntity(new DXFLine(Vector3D(line.x1, line.y1, line.z1),
                        Vector3D(line.x2, line.y2, line.z2)));
}


void DXFReader::addArc(const DL_ArcData &arc) {
  addEntity(new DXFArc(Vector3D(arc.cx, arc.cy, arc.cz), arc.radius, arc.angle1,
                       arc.angle2, arc.clockwise));
}


void DXFReader::addCircle(const DL_CircleData &circle) {
  addEntity(new DXFArc(Vector3D(circle.cx, circle.cy, circle.cz), circle.radius,
                       0, 360, true));
}


void DXFReader::addPolyline(const DL_PolylineData &polyline) {
  if (!entity.isNull()) THROW("DXF Already in DXF entity");
  addEntity(entity = new DXFPolyLine);
}


void DXFReader::addVertex(const DL_VertexData &vertex) {
  if (vertex.bulge) LOG_WARNING("Cannot handle vertex with bulge");
  entity->addVertex(Vector3D(vertex.x, vertex.y, vertex.z));
}


void DXFReader::addSpline(const DL_SplineData &spline) {
  if (!entity.isNull()) THROW("DXF Already in DXF entity");
  addEntity(entity = new DXFSpline(spline.degree));
}


void DXFReader::addControlPoint(const DL_ControlPointData &ctrlPt) {
  entity->addVertex(Vector3D(ctrlPt.x, ctrlPt.y, ctrlPt.z));
}


void DXFReader::addKnot(const DL_KnotData &knot) {
  entity->addKnot(knot.k);
}


void DXFReader::addEllipse(const DL_EllipseData &ellipse) {
  if (warnEllipse) LOG_WARNING("DXF Ellipse not supported");
  warnEllipse = false;
}


void DXFReader::add3dFace(const DL_3dFaceData &face) {
  if (warn3DFace) LOG_WARNING("DXF 3D Face not supported");
  warn3DFace = false;
}


void DXFReader::addSolid(const DL_SolidData &solid) {
  if (warnSolid) LOG_WARNING("DXF Solid not supported");
  warnSolid = false;
}
