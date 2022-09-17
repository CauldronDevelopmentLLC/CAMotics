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
  unsigned size = triangles * 9 * sizeof(float);

  lines.release();
  vertices.allocate(size);
  normals.allocate(size);
}


void Mesh::add(unsigned count, const float *vertices, const float *normals) {
  if (count % 3) THROW("Mesh vertices array size not a multiple of 3");

  lines.release();
  this->vertices.add(3 * count, vertices);
  this->normals.add(3 * count, normals);
}


void Mesh::add(const vector<float> &vertices, const vector<float> &normals) {
  if (vertices.size() != normals.size())
    THROW("Number of vertices not equal to number of normals");
  if (!vertices.empty()) add(vertices.size() / 3, &vertices[0], &normals[0]);
}


void Mesh::glDraw(GLContext &gl) {
  vertices.enable(3);
  normals.enable(3);

  gl.glDrawArrays(GL_TRIANGLES, 0, triangles * 3);

  vertices.disable();
  normals.disable();
}
