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

#include "VBO.h"
#include "GLContext.h"

using namespace CAMotics;


VBO::~VBO() {
  try {
    if (buffer && GLContext::isActive())
      GLContext().glDeleteBuffers(1, &buffer);
  } catch (...) {}
}


unsigned VBO::get() {
  if (!buffer) GLContext().glGenBuffers(1, &buffer);
  return buffer;
}


void VBO::allocate(unsigned size) {
  this->size = size;
  fill = 0;

  GLContext gl;

  gl.glBindBuffer(GL_ARRAY_BUFFER, get());
  gl.glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);
  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VBO::add(unsigned count, const float *data) {
  unsigned bytes = count * sizeof(float);
  unsigned newFill = fill + bytes;
  if (size < newFill) THROW("VBO overflow " << size << " < " << newFill);

  GLContext gl;

  gl.glBindBuffer(GL_ARRAY_BUFFER, get());
  gl.glBufferSubData(GL_ARRAY_BUFFER, fill, bytes, data);
  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  fill = newFill;
}


void VBO::enable(unsigned stride) {
  GLContext gl;

  gl.glEnableVertexAttribArray(id);
  gl.glBindBuffer(GL_ARRAY_BUFFER, get());
  gl.glVertexAttribPointer(id, stride, GL_FLOAT, false, 0, 0);
  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VBO::disable() {GLContext().glDisableVertexAttribArray(id);}
