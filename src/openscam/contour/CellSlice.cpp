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

#include "CellSlice.h"

#include "QEF.h"

using namespace OpenSCAM;

static const int edgeOffsets[12][4] = {
  {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 2},
  {1, 0, 0, 1}, {1, 0, 0, 2}, {0, 1, 0, 0},
  {0, 1, 0, 2}, {0, 0, 1, 0}, {0, 0, 1, 1},
  {1, 1, 0, 2}, {1, 0, 1, 1}, {0, 1, 1, 0},
};


void CellSlice::compute() {
  const HermiteSlice *slices[2] = {&*this->first, &*this->second};

  // Allocate space
  unsigned count = dims.x() * dims.y();
  cellIndices.reserve(count);
  cells.push_back(Cell(0, Vector3R())); // Ignore first index

  for (unsigned y = 0; y < dims.y(); y++)
    for (unsigned x = 0; x < dims.x(); x++) {
      uint8_t signs = slices[0]->getSigns(x, y);
      if (signs == 0 || signs == 255) continue; // No intersection

      Vector3R massPoint;
      real matrixA[12][3];
      real vectorB[12];
      unsigned rows = 0;

      // Compute QEF
      for (unsigned i = 0; i < 12; i++) {
        const int *offset = edgeOffsets[i];
        const Edge *edge =
          slices[offset[2]]->getEdge(x + offset[0], y + offset[1], offset[3]);

        if (edge) {
          for (unsigned j = 0; j < 3; j++)
            matrixA[rows][j] = edge->normal[j];

          vectorB[rows++] = edge->vertex.dotProduct(edge->normal);
          massPoint += edge->vertex;
        }
      }

      if (rows) {
        massPoint /= (real)rows;
        cellIndices.push_back(cells.size());
        cells.push_back(Cell(signs, massPoint +
                             QEF::evaluate(matrixA, vectorB, rows)));

      } else cellIndices.push_back(0); // Empty Cell
    }

  first = second = 0; // Free slices
}
