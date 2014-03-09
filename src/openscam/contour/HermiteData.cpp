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

#include "HermiteData.h"

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void HermiteData::run(FieldFunction &func, const Rectangle3R &bbox, real step) {
  unsigned slices = (bbox.getHeight() + 1) / step;

  Rectangle2R plane(Vector2R(bbox.getMin().x(), bbox.getMin().y()),
                    Vector2R(bbox.getMax().x(), bbox.getMax().y()));
  real zStart = bbox.getMin().z();

  SmartPointer<SampleSlice> first = new SampleSlice(func, plane, zStart, step);
  first->compute();
  SmartPointer<SampleSlice> second;

  for (unsigned i = 0; i < slices && !interrupt; i++) {
    real z = zStart + i * step;
    second = new SampleSlice(func, plane, z, step);
    second->compute();

    push_back(new HermiteSlice(first, second));
    back()->compute();

    first = second;
  }
}
