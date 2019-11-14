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

#include "BoundsView.h"

#include "GL.h"

using namespace cb;
using namespace CAMotics;


void BoundsView::draw() {
  const double v[] = {
    // Top
    rmin.x(), rmin.y(), rmin.z(),
    rmax.x(), rmin.y(), rmin.z(),
    rmax.x(), rmin.y(), rmin.z(),
    rmax.x(), rmin.y(), rmax.z(),
    rmax.x(), rmin.y(), rmax.z(),
    rmin.x(), rmin.y(), rmax.z(),
    rmin.x(), rmin.y(), rmax.z(),
    rmin.x(), rmin.y(), rmin.z(),

    // Bottom
    rmin.x(), rmax.y(), rmin.z(),
    rmax.x(), rmax.y(), rmin.z(),
    rmax.x(), rmax.y(), rmin.z(),
    rmax.x(), rmax.y(), rmax.z(),
    rmax.x(), rmax.y(), rmax.z(),
    rmin.x(), rmax.y(), rmax.z(),
    rmin.x(), rmax.y(), rmax.z(),
    rmin.x(), rmax.y(), rmin.z(),

    // Sides
    rmin.x(), rmin.y(), rmin.z(),
    rmin.x(), rmax.y(), rmin.z(),
    rmax.x(), rmin.y(), rmin.z(),
    rmax.x(), rmax.y(), rmin.z(),
    rmax.x(), rmin.y(), rmax.z(),
    rmax.x(), rmax.y(), rmax.z(),
    rmin.x(), rmin.y(), rmax.z(),
    rmin.x(), rmax.y(), rmax.z(),
  };

  float c[24 * 3];
  for (unsigned i = 0; i < 24 * 3; i++) c[i] = 1;

  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glEnableVertexAttribArray(GL_ATTR_POSITION);
  glFuncs.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_DOUBLE, false, 0, v);

  glFuncs.glEnableVertexAttribArray(GL_ATTR_COLOR);
  glFuncs.glVertexAttribPointer(GL_ATTR_COLOR, 3, GL_FLOAT, false, 0, c);

  glFuncs.glDrawArrays(GL_LINES, 0, 24);

  glFuncs.glDisableVertexAttribArray(GL_ATTR_POSITION);
  glFuncs.glDisableVertexAttribArray(GL_ATTR_COLOR);
}
