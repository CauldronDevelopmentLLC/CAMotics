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

#ifndef OPENSCAM_HERMITE_SLICE_H
#define OPENSCAM_HERMITE_SLICE_H

#include "SampleSlice.h"
#include "Cell.h"
#include "Edge.h"

#include <cbang/StdTypes.h>
#include <cbang/SmartPointer.h>

namespace OpenSCAM {
  class HermiteSlice {
    cb::SmartPointer<SampleSlice> first;
    cb::SmartPointer<SampleSlice> second;
    const cb::Vector2U dims;
    const unsigned width;

    std::vector<uint8_t> signs;
    std::vector<Edge> edges;
    std::vector<uint32_t> edgeIndices[3];

  public:
    HermiteSlice(const cb::SmartPointer<SampleSlice> &first,
                 const cb::SmartPointer<SampleSlice> &second) :
      first(first), second(second), dims(first->getDimensions()),
      width(dims.x() + 1) {}

    const cb::Vector2U &getDimensions() const {return dims;}

    void compute();

    uint8_t getSigns(unsigned x, unsigned y) const
    {return signs[y * width + x];}

    const Edge *getEdge(unsigned x, unsigned y, unsigned edge) const {
      uint32_t index = edgeIndices[edge][y * width + x];
      return index ? &edges[index] : 0;
    }
  };
}

#endif // OPENSCAM_HERMITE_SLICE_H

