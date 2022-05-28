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


#include "Entity.h"

#include <vector>


namespace DXF {
  class PolyLine : public Entity {
    /* Polyline flag (bit-coded; default = 0):
     * 1 = This is a closed polyline (or a polygon mesh closed in the M direction)
     * 2 = Curve-fit vertices have been added
     * 4 = Spline-fit vertices have been added
     * 8 = This is a 3D polyline
     * 16 = This is a 3D polygon mesh
     * 32 = The polygon mesh is closed in the N direction
		 * 64 = The polyline is a polyface mesh
		 * 128 = The linetype pattern is generated continuously around the vertices of this polyline
		 */
    unsigned flags;
    std::vector<cb::Vector3D> vertices;

  public:
    PolyLine(unsigned flags) : flags(flags) {}

    const std::vector<cb::Vector3D> &getVertices() const {return vertices;}

    // From Entity
    void addVertex(const cb::Vector3D &v, double weight = 0.0) {vertices.push_back(v);}
    unsigned getFlags() const {return flags;}
    type_t getType() const {return DXF_POLYLINE;}
  };
}
