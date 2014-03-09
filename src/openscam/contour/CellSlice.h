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

#ifndef OPENSCAM_CELL_SLICE_H
#define OPENSCAM_CELL_SLICE_H

#include "HermiteSlice.h"
#include "Cell.h"

#include <vector>

namespace OpenSCAM {
  class CellSlice {
    cb::SmartPointer<HermiteSlice> first;
    cb::SmartPointer<HermiteSlice> second;
    const cb::Vector2U &dims;
    const unsigned width;

    std::vector<Cell> cells;
    std::vector<uint32_t> cellIndices;

  public:
    CellSlice(const cb::SmartPointer<HermiteSlice> &first,
              const cb::SmartPointer<HermiteSlice> &second) :
      first(first), second(second), dims(first->getDimensions()),
      width(dims.x()) {}

    void compute();

    const Cell *getCell(unsigned x, unsigned y) const {
      uint32_t index = cellIndices[y * width + x];
      return index ? &cells[index] : 0;
    }
  };
}

#endif // OPENSCAM_CELL_SLICE_H

