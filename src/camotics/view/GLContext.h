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

#include "Color.h"
#include "GLEnum.h"
#include "GLUniform.h"
#include "Transform.h"

#include <QOpenGLFunctions>


namespace CAMotics {
  class GLContext : public QOpenGLFunctions {
    GLUniform modelMat;

    std::vector<Transform> stack;

  public:
    GLContext(QOpenGLContext *ctx = 0);

    void setModelMatrix(const GLUniform &m) {modelMat = m;}

    unsigned matrixDepth() const {return stack.size() - 1;}
    void pushMatrix();
    void pushMatrix(const Transform &t);
    void setMatrix(const Transform &t);
    void applyMatrix(const Transform &t);
    void popMatrix();

    void logErrors();

    static bool isActive();

    void setColor(const Color &c);
    void setColor(float r, float g, float b, float a = 1);
  };
}
