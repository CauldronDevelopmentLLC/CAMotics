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

#include "SpheroidSweep.h"

#include "Move.h"

using namespace std;
using namespace OpenSCAM;


SpheroidSweep::SpheroidSweep(real radius, real length) :
  radius(radius), length(length == -1 ? radius : length) {

  if (2 * radius != length) scale = Vector3R(1, 1, 2 * radius / length);
  radius2 = radius * radius;
}


void SpheroidSweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                              vector<Rectangle3R> &bboxes,
                              real tolerance) const {
  Sweep::getBBoxes(start, end, bboxes, radius, length, tolerance);
}


bool SpheroidSweep::contains(const Vector3R &start, const Vector3R &end,
                             const Vector3R &p) const {
  Vector3R s = start + Vector3R(0, 0, length / 2);
  Vector3R e = end + Vector3R(0, 0, length / 2);

  if (2 * radius == length) {
    Vector3R c = Segment3R(s, e).closest(p);
    return p.distanceSquared(c) < radius2;
  }

  // Scale z axis to make spheroids round
  Vector3R pScaled = p * scale;
  Vector3R c = Segment3R(s * scale, e * scale).closest(pScaled);
  return pScaled.distanceSquared(c) < radius2;
}
