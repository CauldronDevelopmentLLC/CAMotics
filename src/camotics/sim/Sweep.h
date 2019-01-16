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

#include <cbang/geom/Rectangle.h>

#include <vector>


namespace GCode {class Move;}

namespace CAMotics {
  class Sweep {
  public:
    virtual ~Sweep() {} // Compiler needs this

    void getBBoxes(const cb::Vector3D &start, const cb::Vector3D &end,
                   std::vector<cb::Rectangle3D> &bboxes, double radius,
                   double length, double zOffset,
                   double tolerance = 0.01) const;

    virtual void getBBoxes(const cb::Vector3D &start, const cb::Vector3D &end,
                           std::vector<cb::Rectangle3D> &bboxes,
                           double tolerance = 0.01) const = 0;
    virtual bool intersects(const GCode::Move &move,
                            const cb::Rectangle3D &box) const {return false;}
    virtual double depth(const cb::Vector3D &start, const cb::Vector3D &end,
                       const cb::Vector3D &p) const = 0;
  };
}
