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

#ifndef OPENSCAM_DXFENTITY_H
#define OPENSCAM_DXFENTITY_H

#include <cbang/Exception.h>
#include <cbang/geom/Vector.h>


namespace OpenSCAM {
  class DXFEntity {
  public:
    typedef enum {
      DXF_POINT,
      DXF_LINE,
      DXF_ARC,
      DXF_POLYLINE,
      DXF_SPLINE,
      DXF_LAST
    } type_t;

    virtual ~DXFEntity() {}

    virtual type_t getType() const = 0;
    virtual void addVertex(const cb::Vector3D &v) {THROW("Cannot add vertex");}
    virtual void addKnot(double k) {THROW("Cannot add knot");}
  };
}

#endif // OPENSCAM_DXFENTITY_H

