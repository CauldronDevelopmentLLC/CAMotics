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

#include "Triangle.h"

using namespace cb;
using namespace CAMotics;


void Triangle::updateNormal() {
  normal = computeNormal();
}


Vector3F Triangle::computeNormal() const {
  return computeNormal(data[0], data[1], data[2]);
}


Vector3F Triangle::computeNormal(const Vector3F &v1, const Vector3F &v2,
                                 const Vector3F &v3) {
  // Compute face normal
  Vector3F n = (v2 - v1).cross(v3 - v1);

  // Normalize
  double length = n.length();
  n /= length;

  return n;
}


Vector3F Triangle::computeNormal(const Triangle3F &t) {
  return computeNormal(t[0], t[1], t[2]);
}
