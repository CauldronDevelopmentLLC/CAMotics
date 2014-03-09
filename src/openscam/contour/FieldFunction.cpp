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

#include "FieldFunction.h"

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Vector3R FieldFunction::getEdgeIntersect(const Vector3R &v1, bool inside1,
                                         const Vector3R &v2, bool inside2) {
  if (inside1 == inside2)
    THROWS("There is no intersection between points " << v1 << " & " << v2);

  Vector3R a = v1;
  Vector3R b = v2;
  Vector3R mid;

  for (unsigned i = 0; i < 10; i++) {
    // Sample mid point
    mid = a + (b - a) * 0.5;
    if (contains(mid) == inside1) a = mid;
    else b = mid;
  }

  return mid;
}
