/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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
  // Compute face normal
  Vector3F n = (data[1] - data[0]).cross(data[2] - data[0]);

  // Normalize
  real length = n.length();
  n /= length;

  return n;
}
