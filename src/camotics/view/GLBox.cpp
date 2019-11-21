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

#include "GLBox.h"

using namespace CAMotics;
using namespace cb;


GLBox::GLBox() {
  const float v[] = {
    // Top
    0, 0, 0,  1, 0, 0,
    1, 0, 0,  1, 0, 1,
    1, 0, 1,  0, 0, 1,
    0, 0, 1,  0, 0, 0,

    // Bottom
    0, 1, 0,  1, 1, 0,
    1, 1, 0,  1, 1, 1,
    1, 1, 1,  0, 1, 1,
    0, 1, 1,  0, 1, 0,

    // Sides
    0, 0, 0,  0, 1, 0,
    1, 0, 0,  1, 1, 0,
    1, 0, 1,  1, 1, 1,
    0, 0, 1,  0, 1, 1,
  };

  GLContext gl;
  gl.glBindBuffer(GL_ARRAY_BUFFER, vbo.get());
  gl.glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * 3, v, GL_STATIC_DRAW);
  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GLBox::setBounds(const Rectangle3D &bounds) {
  Vector3D bMin = bounds.getMin();
  Vector3D bDim = bounds.getDimensions();

  getTransform().toIdentity();
  getTransform().translate(bMin.x(), bMin.y(), bMin.z());
  getTransform().scale(bDim.x(), bDim.y(), bDim.z());
}


void GLBox::glDraw(GLContext &gl) {
  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glBindBuffer(GL_ARRAY_BUFFER, vbo.get());
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, 0);
  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  gl.glDrawArrays(GL_LINES, 0, 24);

  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
}
