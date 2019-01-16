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

#include <QOpenGLFunctions_2_1>

#include <cbang/SStream.h>


#ifdef REAL_IS_FLOAT
#define GL_REAL GL_FLOAT
#else
#define GL_REAL GL_DOUBLE
#endif

namespace CAMotics {
  void checkGLError(const std::string &message = std::string());
  bool haveVBOs();
  void glDisk(double radius, unsigned segments);
  void glCylinder(double base, double top, double height, unsigned segments);
  void glSphere(double radius, unsigned slices, unsigned stacks);

  typedef QOpenGLFunctions_2_1 GLFuncs;
  GLFuncs &getGLFuncs();
}

#ifdef __DEBUG
#define CHECK_GL_ERROR(msg) checkGLError(SSTR(__FUNCTION__ << "() " << msg))
#else
#define CHECK_GL_ERROR(msg)
#endif
