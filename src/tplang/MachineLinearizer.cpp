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

#include "MachineLinearizer.h"

using namespace cb;
using namespace tplang;


void MachineLinearizer::arc(const Vector3D &offset, double angle,
                            plane_t plane) {
  const char *axesNames;
  switch (plane) {
  case XY: axesNames = "XYZ"; break;
  case XZ: axesNames = "XZY"; break;
  case YZ: axesNames = "YZX"; break;
  default: THROWS("Invalid plane: " << plane);
  }

  unsigned char axes[3];
  for (unsigned i = 0; i < 3; i++)
    axes[i] = Axes::toIndex(axesNames[i]);

  Axes current = getPosition();

  // Initial point
  double x = current[axes[0]];
  double y = current[axes[1]];
  double z = current[axes[2]];

  // Center
  Vector2D center(x + offset.x(), y + offset.y());

  // Start angle
  double startAngle =
    Vector2D(-offset.x(), -offset.y()).angleBetween(Vector2D(1, 0));

  // Radius
  double radius = Vector2D(offset.x(), offset.y()).length();

  // Segments
  unsigned segments = (unsigned)(fabs(angle) / (2 * M_PI) * arcPrecision);
  double zDelta = offset.z() / segments;
  double deltaAngle = -angle / segments;

  // Create segments
  for (unsigned i = 0; i < segments; i++) {
    double currentAngle = startAngle + deltaAngle * (i + 1);

    x = center.x() + radius * cos(currentAngle);
    y = center.y() + radius * sin(currentAngle);
    z += zDelta;

    // Move
    current[axes[0]] = x;
    current[axes[1]] = y;
    current[axes[2]] = z;
    move(current, false);
  }
}
