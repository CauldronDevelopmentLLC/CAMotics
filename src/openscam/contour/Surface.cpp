/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "Surface.h"

#include <openscam/view/GL.h>

using namespace OpenSCAM;

Surface::~Surface() {
  if (glDeleteBuffers && vbuf) glDeleteBuffers(1, &vbuf);
}


void Surface::finalize() {
  if (finalized) return;

  if (glGenBuffers) {
    glGenBuffers(1, &vbuf);

    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(float),
                    &normalLines[0], GL_STATIC_DRAW);

    normalLines.clear();
  }

  finalized = true;
}


void Surface::addNormalLine(const Vector3R &vertex, const Vector3R &normal) {
  for (unsigned i = 0; i < 3; i++) normalLines.push_back(vertex[i]);

  Vector3R n = vertex + normal * 2;
  for (unsigned i = 0; i < 3; i++) normalLines.push_back(n[i]);

  normalsCount++;
}


void Surface::drawNormals() {
  finalize();

  glEnableClientState(GL_VERTEX_ARRAY);

  if (glBindBuffer) {
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glVertexPointer(3, GL_FLOAT, 0, 0);

  } else glVertexPointer(3, GL_FLOAT, 0, &normalLines[0]);

  glDrawArrays(GL_LINES, 0, normalsCount * 3);

  glDisableClientState(GL_VERTEX_ARRAY);
}
