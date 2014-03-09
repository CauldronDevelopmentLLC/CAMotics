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

#ifndef OPENSCAM_SAMPLE_SLICE_H
#define OPENSCAM_SAMPLE_SLICE_H

#include "FieldFunction.h"

#include <openscam/Geom.h>

#include <vector>

namespace OpenSCAM {
  class SampleSlice : std::vector<real> {
    FieldFunction &func;
    Rectangle2R bbox;
    cb::Vector2U steps;
    real z;
    real step;

  public:
    SampleSlice(FieldFunction &func, const Rectangle2R &bbox, real z,
                real step);

    void compute();

    FieldFunction &getFunction() const {return func;}
    const Rectangle2R &getBounds() const {return bbox;}
    const cb::Vector2U &getDimensions() const {return steps;}
    real getZ() const {return z;}
    real getStep() const {return step;}

    const real *operator[](unsigned y) const
    {return &std::vector<real>::operator[](y  * (steps.x() + 2));}
    real *operator[](unsigned y)
    {return &std::vector<real>::operator[](y * (steps.x() + 2));}
  };
}

#endif // OPENSCAM_SAMPLE_SLICE_H
