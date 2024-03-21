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
    unsigned flags; ///< See flag_t
    std::vector<cb::Vector3D> vertices;

  public:
    typedef enum {
      POLYLINE_FLAG_CLOSED        = 1 << 0, // Closed polyline or M closed mesh
      POLYLINE_FLAG_CURVE_FIT     = 1 << 1, // Curve-fit vertices added
      POLYLINE_FLAG_SPLINE_FIT    = 1 << 2, // Spline-fit vertices added
      POLYLINE_FLAG_3D_LINE       = 1 << 3, // 3D polyline
      POLYLINE_FLAG_3D_MESH       = 1 << 4, // 3D polygon mesh
      POLYLINE_FLAG_N_CLOSED_MESH = 1 << 5, // Mesh closed in N direction
      POLYLINE_FLAG_FACE_MESH     = 1 << 6, // Polyline is a polyface mesh
      POLYLINE_FLAG_CONTINUOUS    = 1 << 7, // Continuous linetype pattern
    } flag_t;


    PolyLine(unsigned flags) : flags(flags) {}

    unsigned getFlags() const {return flags;}
    bool isClosed() const {return flags & POLYLINE_FLAG_CLOSED;}
    const std::vector<cb::Vector3D> &getVertices() const {return vertices;}

    // From Entity
    void addVertex(const cb::Vector3D &v, double weight = 0)
      {vertices.push_back(v);}
    type_t getType() const {return DXF_POLYLINE;}
  };
}
