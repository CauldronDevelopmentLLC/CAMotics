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

#include "GLDisk.h"

using namespace CAMotics;
using namespace cb;


void GLDisk::glDraw(GLContext &gl) {
  // TODO use VBO
  unsigned count = 1 + segments;
  SmartPointer<double>::Array v = new double[count * 2];

  v[0] = v[1] = 0;

  for (unsigned i = 0; i < segments; i++) {
    float a = i * 2 * M_PI / (segments - 1);
    v[(i + 1) * 2 + 0] = sin(a) * radius;
    v[(i + 1) * 2 + 1] = cos(a) * radius;
  }

  float n[3] = {0, 0, 1};
  gl.glVertexAttrib3fv(GL_ATTR_NORMAL, n);

  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 2, GL_DOUBLE, false, 0, v.get());

  gl.glDrawArrays(GL_TRIANGLE_FAN, 0, count);

  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
}
