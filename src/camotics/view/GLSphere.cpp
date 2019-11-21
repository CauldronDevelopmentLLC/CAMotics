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

using namespace CAMotics;
using namespace cb;


void GLSphere::glDraw(GLContext &gl) {
  // TODO Use VBO
  unsigned count = lngs * 2;
  SmartPointer<double>::Array n = new double[count * 3];
  SmartPointer<double>::Array v = new double[count * 3];

  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_DOUBLE, false, 0, v.get());

  gl.glEnableVertexAttribArray(GL_ATTR_NORMAL);
  gl.glVertexAttribPointer(GL_ATTR_NORMAL, 3, GL_DOUBLE, false, 0, n.get());

  for (unsigned i = 0; i < lats; i++) {
    double lat0 = M_PI * (-0.5 + (double)(i - 1) / (lats - 1));
    double z0 = sin(lat0);
    double zr0 = cos(lat0);

    double lat1 = M_PI * (-0.5 + (double)i / (lats - 1));
    double z1 = sin(lat1);
    double zr1 = cos(lat1);

    for (unsigned j = 0; j < lngs; j++) {
      double lng = 2 * M_PI * (double)(j - 1) / (lngs - 1);
      double x = cos(lng);
      double y = sin(lng);
      unsigned o = j * 3 * 2;

      n[o + 0] = x * zr0; n[o + 1] = y * zr0; n[o + 2] = z0;
      n[o + 3] = x * zr1; n[o + 4] = y * zr1; n[o + 5] = z1;

      v[o + 0] = x * zr0 * radius;
      v[o + 1] = y * zr0 * radius;
      v[o + 2] = z0 * radius;
      v[o + 3] = x * zr1 * radius;
      v[o + 4] = y * zr1 * radius;
      v[o + 5] = z1 * radius;
    }

    gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
  }

  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
  gl.glDisableVertexAttribArray(GL_ATTR_NORMAL);
}
