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

#include "GLUniform.h"
#include "GL.h"

#include <vector>

using namespace CAMotics;
using namespace cb;
using namespace std;


void GLUniform::set(float v0) const {
  getGLFuncs().glUniform1f(location, v0);
}


void GLUniform::set(float v0, float v1) const {
  getGLFuncs().glUniform2f(location, v0, v1);
}



void GLUniform::set(float v0, float v1, float v2) const {
  getGLFuncs().glUniform3f(location, v0, v1, v2);
}



void GLUniform::set(float v0, float v1, float v2, float v3) const {
  getGLFuncs().glUniform4f(location, v0, v1, v2, v3);
}



void GLUniform::set(int v0) const {
  getGLFuncs().glUniform1i(location, v0);
}



void GLUniform::set(int v0, int v1) const {
  getGLFuncs().glUniform2i(location, v0, v1);
}



void GLUniform::set(int v0, int v1, int v2) const {
  getGLFuncs().glUniform3i(location, v0, v1, v2);
}



void GLUniform::set(int v0, int v1, int v2, int v3) const {
  getGLFuncs().glUniform4i(location, v0, v1, v2, v3);
}


void GLUniform::set(const Matrix4x4F &m) const {
  float v[16];

  for (unsigned col = 0; col < 4; col++)
    for (unsigned row = 0; row < 4; row++)
      v[col * 4 + row] = m[row][col];

  getGLFuncs().glUniformMatrix4fv(location, 1, false, v);
}
