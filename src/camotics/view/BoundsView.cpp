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
  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glBegin(GL_LINES);

  // Top
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmin.z());

  // Bottom
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmax.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmin.z());

  // Sides
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmin.z());
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmin.z());
  glFuncs.glVertex3f(rmax.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmax.x(), rmax.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmin.y(), rmax.z());
  glFuncs.glVertex3f(rmin.x(), rmax.y(), rmax.z());

  glFuncs.glEnd();
}
