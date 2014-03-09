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

#ifndef OPENSCAM_ELEMENT_SURFACE_H
#define OPENSCAM_ELEMENT_SURFACE_H

#include "Surface.h"

#include <cbang/SmartPointer.h>

#include <vector>


namespace OpenSCAM {
  class ElementSurface : public Surface {
    unsigned dim;
    bool finalized;

    unsigned vbufs[2];
    std::vector<float> vertices;
    std::vector<float> normals;

    uint64_t count;
    Rectangle3R bounds;

  public:
    ElementSurface(std::vector<cb::SmartPointer<Surface> > &surfaces);
    ElementSurface(unsigned dim);
    virtual ~ElementSurface();

    void finalize();
    void addElement(const Vector3R *vertices);

    // From Surface
    uint64_t getCount() const {return count;}
    Rectangle3R getBounds() const {return bounds;}
    void draw();
    void exportSTL(STL &stl);
    void smooth();
  };
}

#endif // OPENSCAM_ELEMENT_SURFACE_H

