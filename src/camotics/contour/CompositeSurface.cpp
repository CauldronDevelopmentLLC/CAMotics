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

#include "CompositeSurface.h"
#include "TriangleSurface.h"

using namespace cb;
using namespace CAMotics;


void CompositeSurface::add(SmartPointer<Surface> s) {
  if (!s.isNull()) surfaces.push_back(s);
}


SmartPointer<Surface> CompositeSurface::consolidate() {
  if (surfaces.size() == 1) return surfaces[0];
  SmartPointer<Surface> surface = new TriangleSurface(surfaces);
  surfaces.clear();
  add(surface);
  return surface;
}


SmartPointer<Surface> CompositeSurface::copy() const {
  SmartPointer<CompositeSurface> surface = new CompositeSurface;

  for (unsigned i = 0; i < surfaces.size(); i++)
    surface->add(surfaces[i]->copy());

  return surface;
}


uint64_t CompositeSurface::getTriangleCount() const {
  uint64_t triangles = 0;
  for (unsigned i = 0; i < surfaces.size(); i++)
    triangles += surfaces[i]->getTriangleCount();
  return triangles;
}


Rectangle3D CompositeSurface::getBounds() const {
  Rectangle3D bounds;
  for (unsigned i = 0; i < surfaces.size(); i++)
    bounds.add(surfaces[i]->getBounds());
  return bounds;
}


void CompositeSurface::getVertices(vert_cb_t cb) const {
  for (unsigned i = 0; i < surfaces.size(); i++)
    surfaces[i]->getVertices(cb);
}


void CompositeSurface::write(STL::Sink &sink, Task *task) const {
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->write(sink, task);
}


void CompositeSurface::reduce(Task &task) {
  consolidate();
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->reduce(task);
}


void CompositeSurface::read(const JSON::Value &value) {
  THROW("Cannot read JSON with CompositSurface");
}


void CompositeSurface::write(JSON::Sink &sink) const {
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->write(sink);
}
