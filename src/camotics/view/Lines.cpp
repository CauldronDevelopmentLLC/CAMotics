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

#include "Lines.h"

#include <cbang/Exception.h>

using namespace CAMotics;
using namespace std;


Lines::Lines(unsigned size, const float *vertices, const float *colors) :
  size(size), haveColors(colors) {
  if (!size) return;
  if (size % 3) THROW("Vertices array size not a multiple of 3");

  GLContext gl;
  gl.glBindBuffer(GL_ARRAY_BUFFER, this->vertices.get());
  gl.glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices,
                  GL_STATIC_DRAW);

  if (colors) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, this->colors.get());
    gl.glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), colors,
                    GL_STATIC_DRAW);
  }

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Lines::Lines(const vector<float> &vertices, const vector<float> &colors) :
  Lines(vertices.size(), &vertices[0], colors.size() ? &colors[0] : 0) {
  if (colors.size() && vertices.size() != colors.size())
    THROW("Vertices array size much match colors");
}


Lines::Lines(const vector<float> &vertices) :
  Lines(vertices.size(), &vertices[0], 0) {}


void Lines::glDraw(GLContext &gl) {
  // Color
  if (haveColors) {
    gl.glEnableVertexAttribArray(GL_ATTR_COLOR);
    gl.glBindBuffer(GL_ARRAY_BUFFER, colors.get());
    gl.glVertexAttribPointer(GL_ATTR_COLOR, 3, GL_FLOAT, false, 0, 0);
  }

  // Position
  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glBindBuffer(GL_ARRAY_BUFFER, vertices.get());
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, 0);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Draw
  gl.glDrawArrays(GL_LINES, 0, size / 3);

  // Clean up
  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
  if (haveColors) gl.glDisableVertexAttribArray(GL_ATTR_COLOR);
}
