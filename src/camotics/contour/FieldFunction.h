/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include <camotics/Geom.h>

#include <vector>

namespace CAMotics {
  class FieldFunction {
  public:
    virtual ~FieldFunction() {} // Compiler needs this

    Vector3R linearIntersect(Vector3R &a, real &aDepth, Vector3R &b,
                             real &bDepth);
    Vector3R findNormal(Vector3R &a, real aDepth, Vector3R &b, real bDepth);
    bool contains(const Vector3R &p) const {return 0 <= depth(p);}
    bool cull(const Vector3R &p, double offset) const;

    virtual bool cull(const Rectangle3R &r) const {return false;}
    virtual real depth(const Vector3R &p) const = 0;
    virtual Edge getEdge(const Vector3R &v1, real depth1,
                         const Vector3R &v2, real depth2);

    Edge getEdge(const Vector3R &v1, bool inside1,
                 const Vector3R &v2, bool inside2);
  };
}
