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

#ifndef CAMOTICS_CUBE_SLICE_H
#define CAMOTICS_CUBE_SLICE_H

#include "VertexSlice.h"
#include "FieldFunction.h"

#include <cbang/SmartPointer.h>
#include <cbang/StdTypes.h>


namespace CAMotics {
  class CubeSlice {
    Rectangle3R bbox;
    real maxStep;

    cb::Vector2U steps;
    Vector2R step;

    cb::SmartPointer<VertexSlice> left;
    cb::SmartPointer<VertexSlice> right;
    std::vector<std::vector<Edge> > edges[5];

    bool shifted;

  public:
    CubeSlice(const Rectangle3R &bbox, real maxStep);

    const Rectangle3R getBounds() const {return bbox;}

    void compute(FieldFunction &func);
    void shiftZ(real z);
    uint8_t getEdges(unsigned x, unsigned y, Edge edges[12]) const;

  protected:
    bool isInside(int x, int y, int offset[3]) const;
  };
}

#endif // CAMOTICS_CUBE_SLICE_H

