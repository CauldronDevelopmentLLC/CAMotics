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

#ifndef OPENSCAM_GL_H
#define OPENSCAM_GL_H

#include <glew/glew.h> // Must be first

#include <cbang/SStream.h>

#include <openscam/Real.h>

#ifdef REAL_IS_FLOAT
#define GL_REAL GL_FLOAT
#else
#define GL_REAL GL_DOUBLE
#endif

namespace OpenSCAM {
  void checkGLError(const std::string &message = std::string());
}

#ifdef __DEBUG
#define CHECK_GL_ERROR(msg) checkGLError(SSTR(__FUNCTION__ << "() " << msg))
#else
#define CHECK_GL_ERROR(msg)
#endif

#endif // OPENSCAM_GL_H
