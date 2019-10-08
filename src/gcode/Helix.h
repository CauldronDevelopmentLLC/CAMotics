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

#include <cbang/geom/Vector.h>


namespace GCode {
  class Helix {
    cb::Vector3D start;
    cb::Vector3D end;
    double angle;

    cb::Vector2D center;
    double radius;
    double startAngle;
    double offsetAngle;
    double segAngle;
    double zOffset;
    unsigned points;

  public:
    Helix(const cb::Vector3D &start, const cb::Vector2D &centerOffset,
          const cb::Vector3D &end, double angle, double maxError = 0.01);

    unsigned size() const {return points;}
    cb::Vector3D get(unsigned i) const;
  };
}
