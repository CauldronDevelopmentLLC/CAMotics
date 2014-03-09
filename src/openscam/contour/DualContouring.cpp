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

#include "DualContouring.h"

#include "HermiteSlice.h"
#include "CellSlice.h"

using namespace std;
using namespace cb;
using namespace OpenSCAM;

// 0, z, y, x
static const int quadOffsets[16][3][3] = {
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 0000 No intersection
  {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}}, // 0001
  {{0, 0, 1}, {1, 0, 1}, {1, 0, 0}}, // 0010
  {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}}, // 0011
  {{0, 0, 1}, {1, 0, 1}, {1, 0, 0}}, // 0100
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 0101
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 0110
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 0111
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1000
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1001
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1010
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1011
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1100
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1101
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1110
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // 1111 No intersection
};


void DualContouring::run(FieldFunction &func, const Rectangle3R &bbox,
                         real step) {
  surface = new ElementSurface(4);

  Vector3R dims = bbox.getDimensions();
  unsigned slices = (bbox.getHeight() + 1) / step;

  Rectangle2R plane(Vector2R(bbox.getMin().x(), bbox.getMin().y()),
                    Vector2R(bbox.getMax().x(), bbox.getMax().y()));

  real z = bbox.getMin().z();

  SmartPointer<SampleSlice> samples[2];
  SmartPointer<HermiteSlice> hermite[2];
  SmartPointer<CellSlice> cells[2];

  for (unsigned i = 0; i < slices + 2 && !interrupt; i++) {
    samples[1] = new SampleSlice(func, plane, z, step);
    samples[1]->compute();
    z += step;

    if (!samples[0].isNull()) {
      hermite[1] = new HermiteSlice(samples[0], samples[1]);
      hermite[1]->compute();
    }
    samples[0] = samples[1];

    if (!hermite[0].isNull()) {
      cells[1] = new CellSlice(hermite[0], hermite[1]);
      cells[1]->compute();
    }
    hermite[0] = hermite[1];

    if (!cells[0].isNull()) {
      for (unsigned y = 1; y < dims.y(); y++)
        for (unsigned x = 1; x < dims.x(); x++) {
          const Cell *cell = cells[0]->getCell(x, y);
          if (cell) {
            //const int *offset = quadOffsets[cell->signs & 63];
          }
        }
    }
    cells[0] = cells[1];
  }
}
