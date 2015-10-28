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

#include "FieldFunction.h"

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


Vector3R FieldFunction::linearIntersect(Vector3R &a, bool aInside,
                                        Vector3R &b, bool bInside) {
  if (aInside == bInside)
    THROWS("There is no intersection between points " << a << " & " << b);

  Vector3R mid;

  // Binary search for intersection point
  for (unsigned i = 0; i < 8; i++) {
    mid = a + (b - a) * 0.5;
    if (contains(mid) == aInside) a = mid;
    else b = mid;
  }

  return mid;
}


Vector3R FieldFunction::findNormal(Vector3R &a, bool aInside,
                                   Vector3R &b, bool bInside) {
  Vector3R normal;

  // TODO

  return normal;
}


Vector3R FieldFunction::getEdgeIntersect(const Vector3R &v1, bool inside1,
                                         const Vector3R &v2, bool inside2) {
  Vector3R a = v1;
  Vector3R b = v2;
  return linearIntersect(a, inside1, b, inside2);
}


Edge FieldFunction::getEdge(const Vector3R &v1, bool inside1,
                            const Vector3R &v2, bool inside2) {
  Vector3R a = v1;
  Vector3R b = v2;

  Edge e;
  e.vertex = linearIntersect(a, inside1, b, inside2);
  e.normal = findNormal(a, inside1, b, inside2);
  return e;
}
