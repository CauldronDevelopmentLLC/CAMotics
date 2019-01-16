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

#pragma once

#include "Edge.h"

#include <cbang/geom/Vector.h>
#include <cbang/geom/Rectangle.h>

#include <vector>


namespace CAMotics {
  class FieldFunction {
  public:
    virtual ~FieldFunction() {} // Compiler needs this

    cb::Vector3D linearIntersect(cb::Vector3D &a, double &aDepth,
                                 cb::Vector3D &b, double &bDepth);
    cb::Vector3D findNormal(cb::Vector3D &a, double aDepth, cb::Vector3D &b,
                            double bDepth);
    bool contains(const cb::Vector3D &p) const {return 0 <= depth(p);}
    bool cull(const cb::Vector3D &p, double offset) const;

    virtual bool cull(const cb::Rectangle3D &r) const {return false;}
    virtual double depth(const cb::Vector3D &p) const = 0;
    virtual Edge getEdge(const cb::Vector3D &v1, double depth1,
                         const cb::Vector3D &v2, double depth2);

    Edge getEdge(const cb::Vector3D &v1, bool inside1,
                 const cb::Vector3D &v2, bool inside2);
  };
}
