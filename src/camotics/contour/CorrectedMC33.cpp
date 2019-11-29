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

#include "CorrectedMC33.h"
#include "CorrectedMC33Cube.h"

#include "CubeSlice.h"
#include "GridTreeLeaf.h"

using namespace CAMotics;
using namespace cb;


const Vector3D &CorrectedMC33::getCenter(uint8_t index) {
  if (!centerComputed) {
    center = Vector3D();

    unsigned count = 0;
    for (unsigned i = 0; i < 12; i++)
      if (index & (1 << i)) {
        count++;
        center += edges[i].vertex;
      }

    center /= count;
  }

  return center;
}


void CorrectedMC33::doCell(GridTreeRef &tree, const CubeSlice &slice,
                           unsigned x, unsigned y) {
  double data[8];
  uint8_t index = slice.getEdges(x, y, edges, data);
  offset = Vector3U(x, y, slice.getZ());

  CorrectedMC33Cube cube(index, data);
  unsigned count = 0;
  const int8_t *tiling = cube.process(count);
  if (!tiling) return;

  SmartPointer<GridTreeLeaf> leaf = new GridTreeLeaf;
  Triangle t;

  // Draw the triangles that were found.  There can be up to five per cube.
  centerComputed = false;
  for (unsigned i = 0; i < count; i++) {
    for (unsigned j = 0; j < 3; j++) {
      uint8_t edge = tiling[j + i * 3];

      if (edge == 12) t[j] = getCenter(index);
      else t[j] = edges[edge].vertex;
    }

    t.updateNormal();
    leaf->add(t);
  }

  tree.insertLeaf(leaf.adopt(), offset);
}
