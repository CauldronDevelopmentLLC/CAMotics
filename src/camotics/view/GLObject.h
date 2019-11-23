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

#include "GLContext.h"
#include "Color.h"
#include "Transform.h"


namespace CAMotics {
  class GLObject {
    Transform t;
    Color color;
    bool light = true;
    bool visible = true;

  public:
    virtual ~GLObject() {}

    const Transform &getTransform() const {return t;}
    Transform &getTransform() {return t;}

    const Color &getColor() const {return color;}
    Color &getColor() {return color;}
    void setColor(const Color &color) {this->color = color;}
    void setColor(float r, float g, float b, float a = 1)
      {setColor(Color(r, g, b, a));}

    bool getLight() const {return light;}
    void setLight(bool light) {this->light = light;}

    void setVisible(bool visible) {this->visible = visible;}
    bool isVisible() const {return visible;}

    virtual void glDraw(GLContext &gl) = 0;
  };
}
