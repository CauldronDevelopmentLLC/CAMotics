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

#include "VertexSlice.h"

#include <limits>

using namespace std;
using namespace cb;
using namespace CAMotics;


VertexSlice::VertexSlice(const GridTreeRef &grid, unsigned z) :
  grid(grid), z(z) {}


void VertexSlice::compute(FieldFunction &func) {
  // Allocate space
  const Vector3U &steps = grid.getSteps();
  resize(steps.x() + 1,
         vector<double>(steps.y() + 1, -numeric_limits<double>::max()));

  double resolution = grid.getResolution();
  Vector3D p = Vector3D(0, 0, grid.getOffset().z() + resolution * z);

  for (unsigned x = 0; x <= steps.x(); x++) {
    p.x() = grid.getOffset().x() + resolution * x;

    for (unsigned y = 0; y <= steps.y(); y++) {
      p.y() = grid.getOffset().y() + resolution * y;

      if (!func.cull(p, 2.1 * resolution)) at(x).at(y) = func.depth(p);
    }
  }
}
