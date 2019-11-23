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
    unsigned vertFill = 0;
    bool withColors   = false;

    VBO vertices = GL_ATTR_POSITION;
    VBO colors   = GL_ATTR_COLOR;

  public:
    Lines(unsigned lines, bool withColors);
    Lines(unsigned count, const float *vertices, const float *colors = 0);
    Lines(const std::vector<float> &vertices, const std::vector<float> &colors);
    Lines(const std::vector<float> &vertices);

    void reset(unsigned lines, bool withColors = false);

    void add(unsigned lines, const float *vertices, const float *colors = 0);
    void add(const std::vector<float> &vertices,
             const std::vector<float> &colors);
    void add(const std::vector<float> &vertices);

    // From GLObject
    void glDraw(GLContext &gl);
  };
}
