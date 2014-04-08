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

#include "Sweep.h"

#include "Move.h"

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void Sweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                      vector<Rectangle3R> &bboxes, real radius,
                      real length, real tolerance) const {
  const unsigned maxLen = radius * 16;
  real len = start.distance(end);
  unsigned steps = (len <= maxLen) ? 1 : (len / maxLen);
  real stride = 1.0 / steps;
  Vector3R p1 = start;
  Vector3R p2;

  for (unsigned i = 0; i < steps; i++) {
    for (unsigned j = 0; j < 3; j++)
      p2[j] = start[j] + (end[j] - start[j]) * stride * (i + 1);

    real minX = std::min(p1.x(), p2.x()) - radius - tolerance;
    real minY = std::min(p1.y(), p2.y()) - radius - tolerance;
    real minZ = std::min(p1.z(), p2.z()) - tolerance;
    real maxX = std::max(p1.x(), p2.x()) + radius + tolerance;
    real maxY = std::max(p1.y(), p2.y()) + radius + tolerance;
    real maxZ = std::max(p1.z(), p2.z()) + length + tolerance;

    bboxes.push_back
      (Rectangle3R(Vector3R(minX, minY, minZ), Vector3R(maxX, maxY, maxZ)));

    p1 = p2;
  }
}


bool Sweep::contains(const Move &move, const Vector3R &p, double time) const {
  return contains(move.getStartPt(), time < move.getEndTime() ?
                  move.getEndPtAtTime(time) : move.getEndPt(), p);
}
