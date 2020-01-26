/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include <cmath>

using namespace std;
using namespace cb;
using namespace CAMotics;


Vector3D FieldFunction::linearIntersect(Vector3D &a, double &aDepth,
                                        Vector3D &b, double &bDepth) {
  if ((aDepth < 0) == (bDepth < 0))
    THROW("There is no intersection between points " << a << " & " << b);

  Vector3D mid;

  // Binary search for intersection point
  for (unsigned i = 0; i < 8; i++) {
    mid = a + (b - a) * 0.5;
    double midDepth = depth(mid);

    if ((midDepth < 0) == (aDepth < 0)) {
      a = mid;
      aDepth = midDepth;

    } else {
      b = mid;
      bDepth = midDepth;
    }
  }

  return mid;
}


Vector3D FieldFunction::findNormal(Vector3D &a, double aDepth,
                                   Vector3D &b, double bDepth) {
  Vector3D normal;

  // TODO Find normal

  return normal;
}


bool FieldFunction::cull(const Vector3D &p, double offset) const {
  return cull(Rectangle3D(p, p).grow(offset));
}


Edge FieldFunction::getEdge(const Vector3D &v1, double depth1,
                            const Vector3D &v2, double depth2) {
  Vector3D a = v1;
  Vector3D b = v2;

  Edge e;
  e.vertex = linearIntersect(a, depth1, b, depth2);
  e.normal = findNormal(a, depth1, b, depth2);
  return e;

}


Edge FieldFunction::getEdge(const Vector3D &v1, bool inside1,
                            const Vector3D &v2, bool inside2) {
  return
    getEdge(v1, (double)(inside1 ? 1 : -1), v2, (double)(inside2 ? 1 : -1));
}
