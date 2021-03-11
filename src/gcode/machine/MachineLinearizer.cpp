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

#include "MachineLinearizer.h"

#include <gcode/Helix.h>

using namespace cb;
using namespace GCode;


void MachineLinearizer::arc(const Vector3D &offset, const Vector3D &target,
                            double angle, plane_t plane) {
  const char *axesNames;
  switch (plane) {
  case XY: axesNames = "XYZ"; break;
  case XZ: axesNames = "XZY"; break;
  case YZ: axesNames = "YZX"; break;
  default: THROW("Invalid plane: " << plane);
  }

  // Get axis indices
  unsigned axisIndex[3];
  for (unsigned i = 0; i < 3; i++)
    axisIndex[i] = Axes::toIndex(axesNames[i]);

  // Offset point
  double xOff = offset[axisIndex[0]];
  double yOff = offset[axisIndex[1]];
  double zOff = offset[axisIndex[2]];

  // Initial point
  Axes current = getPosition();
  Vector3D start(current[axisIndex[0]],
                 current[axisIndex[1]],
                 current[axisIndex[2]]);
  Vector3D end(target[axisIndex[0]],
               target[axisIndex[1]],
               target[axisIndex[2]]);

  // Axis vars
  int axes = getVarType(axesNames[0]) | getVarType(axesNames[1]);
  if (zOff) axes |= getVarType(axesNames[2]);

  double maxArcError = get("_max_arc_error", Units::METRIC);
  Helix helix(start, Vector2D(xOff, yOff), end, angle, maxArcError);

  // Create segments
  for (unsigned i = 1; i < helix.size() - 1; i++) {
    Vector3D p = helix.get(i);

    // Move
    current[axisIndex[0]] = p.x();
    current[axisIndex[1]] = p.y();
    current[axisIndex[2]] = p.z();
    move(current, axes, false, 0);
  }

  // Last segment, move to target exactly
  current.setXYZ(target);
  move(current, axes, false, 0);
}
