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

#include "GLSphere.h"

#include <vector>

using namespace CAMotics;
using namespace cb;
using namespace std;


namespace {
  float getLat(float ratio, bool hemi) {
    return M_PI * (hemi ? -0.5 * ratio : (-0.5 + ratio));
  }
}


GLSphere::GLSphere(float radius, unsigned lats, unsigned lngs, bool hemi) :
  lats(lats), lngs(lngs) {

  unsigned count = lats * lngs * 6;
  vector<float> n(count);
  vector<float> v(count);

  for (unsigned i = 0; i < lats; i++) {
    float lat0 = getLat((float)i / lats, hemi);
    float z0   = sin(lat0);
    float zr0  = cos(lat0);

    float lat1 = getLat((float)(i + 1) / lats, hemi);
    float z1   = sin(lat1);
    float zr1  = cos(lat1);

    for (unsigned j = 0; j < lngs; j++) {
      float lng  = 2 * M_PI * (float)j / (lngs - 1);
      float x    = cos(lng);
      float y    = sin(lng);
      unsigned o = ((i * lngs) + j) * 6;

      n[o + 0] = x * zr0; n[o + 1] = y * zr0; n[o + 2] = z0;
      n[o + 3] = x * zr1; n[o + 4] = y * zr1; n[o + 5] = z1;

      v[o + 0] = x * zr0 * radius;
      v[o + 1] = y * zr0 * radius;
      v[o + 2] = z0 * radius;
      v[o + 3] = x * zr1 * radius;
      v[o + 4] = y * zr1 * radius;
      v[o + 5] = z1 * radius;
    }
  }

  vertices.allocate(count * sizeof(float));
  normals.allocate(count  * sizeof(float));
  vertices.add(count, v.data());
  normals.add(count,  n.data());
}


void GLSphere::glDraw(GLContext &gl) {

  vertices.enable(3);
  normals.enable(3);
  gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, lats * lngs * 2);
  vertices.disable();
  normals.disable();
}
