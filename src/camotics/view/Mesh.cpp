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

#include "Mesh.h"

#include <cbang/Exception.h>

using namespace CAMotics;
using namespace std;


Mesh::Mesh(unsigned triangles) {reset(triangles);}


void Mesh::reset(unsigned triangles) {
  this->triangles = triangles;
  fill = 0;

  unsigned size = triangles * 9 * sizeof(float);
  GLContext gl;

  // Vertices
  gl.glBindBuffer(GL_ARRAY_BUFFER, vertices.get());
  gl.glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

  // Normals
  gl.glBindBuffer(GL_ARRAY_BUFFER, normals.get());
  gl.glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Mesh::add(unsigned count, const float *vertices, const float *normals) {
  if (count % 3) THROW("Mesh vertices array size not a multiple of 3");
  if (triangles * 3 < count + fill)
    THROW("Mesh buffer overflow " << (triangles * 3) << " < "
          << (count + fill));

  unsigned size   = sizeof(float) * count * 3;
  unsigned offset = sizeof(float) * fill * 3;
  fill += count;

  GLContext gl;

  // Vertices
  gl.glBindBuffer(GL_ARRAY_BUFFER, this->vertices.get());
  gl.glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices);

  // Normals
  gl.glBindBuffer(GL_ARRAY_BUFFER, this->normals.get());
  gl.glBufferSubData(GL_ARRAY_BUFFER, offset, size, normals);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Mesh::add(const vector<float> &vertices, const vector<float> &normals) {
  if (vertices.size() != normals.size())
    THROW("Number of vertices not equal to number of normals");
  add(vertices.size() / 3, &vertices[0], &normals[0]);
}


void Mesh::glDraw(GLContext &gl) {
  // Vertices
  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glBindBuffer(GL_ARRAY_BUFFER, vertices.get());
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, 0);

  // Normals
  gl.glEnableVertexAttribArray(GL_ATTR_NORMAL);
  gl.glBindBuffer(GL_ARRAY_BUFFER, normals.get());
  gl.glVertexAttribPointer(GL_ATTR_NORMAL, 3, GL_FLOAT, false, 0, 0);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  gl.glDrawArrays(GL_TRIANGLES, 0, triangles * 3);

  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
  gl.glDisableVertexAttribArray(GL_ATTR_NORMAL);
}
