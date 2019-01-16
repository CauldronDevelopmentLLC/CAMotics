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

#include "Sweep.h"

#include <gcode/Move.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


void Sweep::getBBoxes(const Vector3D &start, const Vector3D &end,
                      vector<Rectangle3D> &bboxes, double radius,
                      double length, double zOffset, double tolerance) const {
  const unsigned maxLen = radius * 16;
  double len = start.distance(end);
  unsigned steps = (len <= maxLen) ? 1 : (len / maxLen);
  double stride = 1.0 / steps;
  Vector3D p1 = start;
  Vector3D p2;

  for (unsigned i = 0; i < steps; i++) {
    for (unsigned j = 0; j < 3; j++)
      p2[j] = start[j] + (end[j] - start[j]) * stride * (i + 1);

    double minX = std::min(p1.x(), p2.x()) - radius - tolerance;
    double minY = std::min(p1.y(), p2.y()) - radius - tolerance;
    double minZ = std::min(p1.z(), p2.z()) + zOffset - tolerance;
    double maxX = std::max(p1.x(), p2.x()) + radius + tolerance;
    double maxY = std::max(p1.y(), p2.y()) + radius + tolerance;
    double maxZ = std::max(p1.z(), p2.z()) + length + tolerance;

    bboxes.push_back
      (Rectangle3D(Vector3D(minX, minY, minZ),
                       Vector3D(maxX, maxY, maxZ)));

    p1 = p2;
  }
}
