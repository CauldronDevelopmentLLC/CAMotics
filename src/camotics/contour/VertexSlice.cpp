/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "VertexSlice.h"

#include <limits>

using namespace std;
using namespace cb;
using namespace CAMotics;


VertexSlice::VertexSlice(const Rectangle2R &bbox, real maxStep, real z) :
  bbox(bbox), z(z) {
  // Compute steps and step
  steps = (bbox.getDimensions() / maxStep).ceil();
  step = bbox.getDimensions() / steps;
}


void VertexSlice::compute(FieldFunction &func) {
  // Allocate space
  resize(steps.x() + 1, vector<bool>(steps.y() + 1, false));

  Vector3R p = Vector3R(0, 0, z);

  for (unsigned x = 0; x <= steps.x(); x++) {
    p.x() = bbox.rmin.x() + step.x() * x;

    for (unsigned y = 0; y <= steps.y(); y++) {
      p.y() = bbox.rmin.y() + step.y() * y;

      at(x).at(y) = func.contains(p);
    }
  }
}
