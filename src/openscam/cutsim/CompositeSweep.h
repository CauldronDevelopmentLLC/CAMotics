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

#ifndef OPENSCAM_COMPOSITE_SWEEP_H
#define OPENSCAM_COMPOSITE_SWEEP_H

#include "Sweep.h"

#include <cbang/SmartPointer.h>

#include <vector>


namespace OpenSCAM {
  class CompositeSweep : public Sweep {
    std::vector<cb::SmartPointer<Sweep> > children;
    std::vector<real> zOffsets;

  public:
    void add(const cb::SmartPointer<Sweep> &sweep, real zOffset = 0);

    // From Sweep
    void getBBoxes(const Vector3R &start, const Vector3R &end,
                   std::vector<Rectangle3R> &bboxes,
                   real tolerance = 0.01) const;
    bool contains(const Vector3R &start, const Vector3R &end,
                  const Vector3R &p) const;
  };
}

#endif // OPENSCAM_COMPOSITE_SWEEP_H

