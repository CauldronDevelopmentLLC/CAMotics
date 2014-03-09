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

#include "SampleSlice.h"

#include <limits>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


SampleSlice::SampleSlice(FieldFunction &func, const Rectangle2R &bbox,
                         real z, real step) :
  func(func), bbox(bbox), z(z), step(step) {

  // Compute steps
  steps = (bbox.getDimensions() + Vector2R(1)) / step;
}


void SampleSlice::compute() {
  // Allocate space
  resize((steps[0] + 2) * (steps[1] + 2), -std::numeric_limits<real>::max());

  // Compute offsets
  const Vector2R &rmin = bbox.getMin();
  Vector2R scale = Vector2R(bbox.getWidth() / steps.x(),
                            bbox.getLength() / steps.y());
  Vector3R p = Vector3R(0, 0, z);

  // TODO Culling?

  // Sample
  //real *array = (*this)[0];
  for (unsigned y = 0; y < steps.y() + 2; y++) {
    p.y() = rmin.y() + y * scale.y();

    for (unsigned x = 0; x < steps.x() + 2; x++) {
      p.x() = rmin.x() + x * scale.x();

      // TODO func.getSample() was removed
      //*array++ = func.getSample(p);
    }
  }
}
