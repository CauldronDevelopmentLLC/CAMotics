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


Lines::Lines(unsigned lines, bool withColors, bool withNormals) {
  reset(lines, withColors, withNormals);
}


Lines::Lines(unsigned count, const float *vertices, const float *colors,
             const float *normals) {
  reset(count / 3, colors, normals);
  add(lines, vertices, colors, normals);
}


Lines::Lines(const vector<float> &vertices, const vector<float> &colors,
             const vector<float> &normals) {
  reset(vertices.size() / 3, !colors.empty(), !normals.empty());
  add(vertices, colors, normals);
}


Lines::Lines(const vector<float> &vertices, const vector<float> &colors) {
  reset(vertices.size() / 3, !colors.empty());
  add(vertices, colors);
}


Lines::Lines(const vector<float> &vertices) {
  reset(vertices.size() / 3, false);
  add(vertices);
}


void Lines::reset(unsigned lines, bool withColors, bool withNormals) {
  this->lines = lines;
  this->withColors = withColors;
  this->withNormals = withNormals;
  setLight(withNormals);

  unsigned size = lines * 6 * sizeof(float);
  vertices.allocate(size);
  if (withColors) colors.allocate(size);
  if (withNormals) normals.allocate(size);
}


void Lines::add(unsigned count, const float *vertices, const float *colors,
                const float *normals) {
  if (count % 2) THROW("Lines vertices array size not a multiple of 2");
  if (colors) {
    if (!withColors) THROW("Cannot add colors");
  } else if (withColors) THROW("Missing colors");
  if (normals) {
    if (!withNormals) THROW("Cannot add normals");
  } else if (withNormals) THROW("Missing normals");

  this->vertices.add(3 * count, vertices);
  if (colors) this->colors.add(3 * count, colors);
  if (normals) this->normals.add(3 * count, normals);
}


void Lines::add(const vector<float> &vertices, const vector<float> &colors,
                const vector<float> &normals) {
  if (colors.size() && vertices.size() != colors.size())
    THROW("Vertices array size must match colors");
  if (normals.size() && vertices.size() != normals.size())
    THROW("Vertices array size must match normals");

  add(vertices.size() / 3, &vertices[0], &colors[0], &normals[0]);
}


void Lines::add(const vector<float> &vertices, const vector<float> &colors) {
  if (colors.size() && vertices.size() != colors.size())
    THROW("Vertices array size must match colors");
  add(vertices.size() / 3, &vertices[0], &colors[0]);
}


void Lines::add(const vector<float> &vertices) {
  add(vertices.size() / 3, &vertices[0]);
}


void Lines::glDraw(GLContext &gl) {
  if (!lines) return;

  vertices.enable(3);
  if (withColors) colors.enable(3);
  if (withNormals) normals.enable(3);

  gl.glDrawArrays(GL_LINES, 0, lines * 2);

  vertices.disable();
  if (withColors) colors.disable();
  if (withNormals) normals.disable();
}
