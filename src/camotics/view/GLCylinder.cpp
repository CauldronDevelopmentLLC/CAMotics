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

#include "GLCylinder.h"

using namespace CAMotics;
using namespace cb;

GLCylinder::GLCylinder(float base, float top, float height, unsigned segments) :
  base(base), top(top), height(height), segments(segments) {

  const unsigned count = segments * 6;
  float v[count];
  float n[count];

  float b = sqrt((base - top) * (base - top) + height * height);

  for (unsigned i = 0; i < segments; i++) {
    unsigned o = i * 3 * 2;
    float a = i * 2 * M_PI / (segments - 1);

    // Normals
    n[o + 0] = n[o + 3] = cos(a) * height / b;
    n[o + 1] = n[o + 4] = sin(a) * height / b;
    n[o + 2] = n[o + 5] = (base - top) / b;

    // Vertices
    v[o + 0] = base * cos(a);
    v[o + 1] = base * sin(a);
    v[o + 2] = 0;
    v[o + 3] = top * cos(a);
    v[o + 4] = top * sin(a);
    v[o + 5] = height;
  }

  vertices.allocate(count * sizeof(float));
  normals.allocate(count  * sizeof(float));
  vertices.add(count, v);
  normals.add(count,  n);
}


void GLCylinder::glDraw(GLContext &gl) {
  vertices.enable(3);
  normals.enable(3);
  gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, segments * 2);
  vertices.disable();
  normals.disable();
}
