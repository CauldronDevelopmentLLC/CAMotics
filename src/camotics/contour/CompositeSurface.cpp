/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#include "CompositeSurface.h"
#include "ElementSurface.h"

using namespace cb;
using namespace CAMotics;


void CompositeSurface::add(SmartPointer<Surface> s) {
  if (!s.isNull()) surfaces.push_back(s);
}


cb::SmartPointer<Surface> CompositeSurface::consolidate() {
  if (surfaces.size() == 1) return surfaces[0];
  cb::SmartPointer<Surface> surface = new ElementSurface(surfaces);
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


uint64_t CompositeSurface::getCount() const {
  uint64_t triangles = 0;
  for (unsigned i = 0; i < surfaces.size(); i++)
    triangles += surfaces[i]->getCount();
  return triangles;
}


Rectangle3R CompositeSurface::getBounds() const {
  Rectangle3R bounds;
  for (unsigned i = 0; i < surfaces.size(); i++)
    bounds.add(surfaces[i]->getBounds());
  return bounds;
}


void CompositeSurface::draw() {
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->draw();
}


void CompositeSurface::write(STL &stl) {
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->write(stl);
}


void CompositeSurface::reduce(Task &task) {
  consolidate();
  for (unsigned i = 0; i < surfaces.size(); i++) surfaces[i]->reduce(task);
}
