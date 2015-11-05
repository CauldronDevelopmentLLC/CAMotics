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

#include "SliceContourGenerator.h"

using namespace cb;
using namespace CAMotics;


void SliceContourGenerator::run(FieldFunction &func, const Grid &grid) {
  surface = new TriangleSurface;

  // Progress
  unsigned completedCells = 0;
  unsigned totalCells = grid.getTotalCells();

  // Compute slices
  const Vector3U &steps = grid.getSteps();
  CubeSlice slice(grid);

  for (unsigned z = 0; !shouldQuit() && z < steps.z(); z++) {
    if (z) slice.shift();
    slice.compute(func);
    doSlice(func, slice, z);

    for (unsigned y = 0; y < steps.y(); y++) {
      for (unsigned x = 0; x < steps.x(); x++) {
        doCell(slice, x, y);

        // Progress
        if ((++completedCells & 7) == 0)
          updateProgress((double)completedCells / totalCells);
      }
    }
  }
}
