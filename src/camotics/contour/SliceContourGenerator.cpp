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

#include "SliceContourGenerator.h"
#include "TriangleSurface.h"

using namespace cb;
using namespace CAMotics;


void SliceContourGenerator::run(FieldFunction &func, GridTreeRef &grid) {
  // Progress
  unsigned completedCells = 0;
  unsigned totalCells = grid.getTotalCells();

  // Compute slices
  const Vector3U &steps = grid.getSteps();
  CubeSlice slice(grid);
  double resolution = grid.getResolution();
  Vector3D p;

  Task::begin("Contouring cut surface");

  for (unsigned z = 0; z < steps.z(); z++) {
    p.z() = grid.getOffset().z() + resolution * z;

    if (z) slice.shift();
    slice.compute(*this, func);
    doSlice(func, slice, z);

    for (unsigned y = 0; y < steps.y(); y++) {
      p.y() = grid.getOffset().y() + resolution * y;

      for (unsigned x = 0; x < steps.x(); x++) {
        p.x() = grid.getOffset().x() + resolution * x;

        if (!func.cull(p, resolution * 1.1)) doCell(grid, slice, x, y);

        // Progress
        if ((++completedCells & 7) == 0 &&
            !Task::update((double)completedCells / totalCells)) return;
      }
    }
  }
}
