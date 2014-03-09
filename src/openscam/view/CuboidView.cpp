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

#include "CuboidView.h"

#include "GL.h"

using namespace OpenSCAM;


CuboidView::~CuboidView() {
  if (glDeleteBuffers) {
    if (vertexVBuf) glDeleteBuffers(1, &vertexVBuf);
    if (normalVBuf) glDeleteBuffers(1, &normalVBuf);
  }
}


void CuboidView::draw() {
  if (bounds == Rectangle3R()) return;

  static float vertices[] = {
    1, 0, 0,  1, 0, 1,  1, 1, 1,  1, 1, 0,
    0, 0, 0,  0, 0, 1,  0, 1, 1,  0, 1, 0,

    0, 1, 0,  1, 1, 0,  1, 1, 1,  0, 1, 1,
    0, 0, 0,  1, 0, 0,  1, 0, 1,  0, 0, 1,

    0, 0, 1,  1, 0, 1,  1, 1, 1,  0, 1, 1,
    0, 0, 0,  1, 0, 0,  1, 1, 0,  0, 1, 0,
  };

  static float normals[] = {
     1,  0,  0,   1,  0,  0,   1,  0,  0,   1,  0,  0,
    -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  -1,  0,  0,

     0,  1,  0,   0,  1,  0,   0,  1,  0,   0,  1,  0,
     0, -1,  0,   0, -1,  0,   0, -1,  0,   0, -1,  0,

     0,  0,  1,   0,  0,  1,   0,  0,  1,   0,  0,  1,
     0,  0, -1,   0,  0, -1,   0,  0, -1,   0,  0, -1,
  };

  if (glGenBuffers) {
    if (!vertexVBuf) {
      glGenBuffers(1, &vertexVBuf);
      glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf);
      glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float),
                   vertices, GL_STATIC_DRAW);

      glGenBuffers(1, &normalVBuf);
      glBindBuffer(GL_ARRAY_BUFFER, normalVBuf);
      glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float),
                   normals, GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBuf);
    glNormalPointer(GL_FLOAT, 0, 0);

  } else {
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);
  }

  glEnable(GL_NORMALIZE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glPushMatrix();

  Vector3R bMin = bounds.getMin();
  Vector3R bDim = bounds.getDimensions();
  glTranslated(bMin.x(), bMin.y(), bMin.z());
  glScaled(bDim.x(), bDim.y(), bDim.z());

  glDrawArrays(GL_QUADS, 0, 24);

  glPopMatrix();

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}
