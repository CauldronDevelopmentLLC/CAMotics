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

#ifndef CAMOTICS_SLICE_CONTOUR_GENERATOR_H
#define CAMOTICS_SLICE_CONTOUR_GENERATOR_H

#include "ContourGenerator.h"
#include "TriangleSurface.h"
#include "CubeSlice.h"


namespace CAMotics {
  class SliceContourGenerator : public ContourGenerator {
  protected:
    cb::SmartPointer<TriangleSurface> surface;

  public:
    virtual void doSlice(FieldFunction &func, const CubeSlice &slice,
                         unsigned z) {}
    virtual void doCell(const CubeSlice &slice, unsigned x, unsigned y) = 0;

    // From ContourGenerator
    cb::SmartPointer<Surface> getSurface() {return surface;}
    void run(FieldFunction &func, const Grid &grid);
  };
}

#endif // CAMOTICS_SLICE_CONTOUR_GENERATOR_H

