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

#include "Arc.h"

using namespace GCode;
using namespace cb;


Vector3D Arc::getTarget() const {
  const unsigned *axisIndex = plane.getAxisIndex();

  // Offset point
  double xOff = offset[axisIndex[0]];
  double yOff = offset[axisIndex[1]];
  double zOff = offset[axisIndex[2]];

  // Initial point
  double x = start[axisIndex[0]];
  double y = start[axisIndex[1]];
  double z = start[axisIndex[2]];

  // Angles
  double startAngle = Vector2D(-xOff, -yOff).angleBetween(Vector2D(1, 0));
  double endAngle = startAngle + angle;

  // Radius
  double radius = Vector2D(xOff, yOff).length();

  // Target
  Vector3D target;
  target[axisIndex[0]] = x + xOff + radius * cos(endAngle);
  target[axisIndex[1]] = y + yOff + radius * sin(endAngle);
  target[axisIndex[2]] = z + zOff;

  return target;
}
