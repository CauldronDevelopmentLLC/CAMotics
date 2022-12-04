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

#include "GLObject.h"
#include "GLEnum.h"
#include "VBO.h"

#include <vector>


namespace CAMotics {
  class Lines : public GLObject {
    unsigned lines    = 0;
    bool withColors   = false;
    bool withNormals  = false;

    VBO vertices = GL_ATTR_POSITION;
    VBO colors   = GL_ATTR_COLOR;
    VBO normals  = GL_ATTR_NORMAL;

  public:
    Lines(unsigned lines, bool withColors, bool withNormals);
    Lines(unsigned count, const float *vertices, const float *colors = 0,
          const float *normals = 0);
    Lines(const std::vector<float> &vertices, const std::vector<float> &colors,
          const std::vector<float> &normals);
    Lines(const std::vector<float> &vertices, const std::vector<float> &colors);
    Lines(const std::vector<float> &vertices);

    bool empty() const {return !lines;}

    void reset(unsigned lines, bool withColors = false,
               bool withNormals = false);

    void add(unsigned lines, const float *vertices, const float *colors = 0,
             const float *normals = 0);
    void add(const std::vector<float> &vertices,
             const std::vector<float> &colors,
             const std::vector<float> &normals);
    void add(const std::vector<float> &vertices,
             const std::vector<float> &colors);
    void add(const std::vector<float> &vertices);

    // From GLObject
    void glDraw(GLContext &gl);
  };
}
