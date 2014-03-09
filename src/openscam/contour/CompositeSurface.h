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

#ifndef OPENSCAM_COMPOSITE_SURFACE_H
#define OPENSCAM_COMPOSITE_SURFACE_H

#include "Surface.h"

#include <cbang/SmartPointer.h>

#include <vector>

namespace OpenSCAM {
  class CompositeSurface : public Surface {
    typedef std::vector<cb::SmartPointer<Surface> > surfaces_t;
    surfaces_t surfaces;

  public:
    void add(cb::SmartPointer<Surface> s);

    typedef surfaces_t::const_iterator iterator;
    iterator begin() const {return surfaces.begin();}
    iterator end() const {return surfaces.end();}

    cb::SmartPointer<Surface> collect();

    // From Surface
    uint64_t getCount() const;
    Rectangle3R getBounds() const;
    void draw();
    void drawNormals();
    void exportSTL(STL &stl);
    void smooth();
  };
}

#endif // OPENSCAM_COMPOSITE_SURFACE_H

