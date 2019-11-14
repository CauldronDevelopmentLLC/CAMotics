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

#pragma once

#include <QOpenGLFunctions>

#include <cbang/SStream.h>


#ifdef REAL_IS_FLOAT
#define GL_REAL GL_FLOAT
#else
#define GL_REAL GL_DOUBLE
#endif

namespace CAMotics {
  void logGLErrors();

  QOpenGLContext &getGLCtx();

  typedef QOpenGLFunctions GLFuncs;
  GLFuncs &getGLFuncs();

  void glDisk(double radius, unsigned segments);
  void glCylinder(double base, double top, double height, unsigned segments);
  void glConic(double radiusA, double radiusB, double length,
               unsigned segments = 128);
  void glSphere(double radius, unsigned slices, unsigned stacks);


  enum {
    GL_ATTR_POSITION,
    GL_ATTR_COLOR,
  };
}
