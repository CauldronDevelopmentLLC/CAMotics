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
#include "GLProgram.h"
#include "GLComposite.h"

#include <cbang/SmartPointer.h>
#include <cbang/geom/Rectangle.h>
#include <cbang/geom/Quaternion.h>

#include <vector>


namespace CAMotics {
  class GLScene : public GLComposite {
    unsigned width  = 0;
    unsigned height = 0;

    double zoom = 1;
    cb::QuaternionD rotation;
    cb::Vector2D translation;
    cb::Rectangle3D bbox;
    cb::Vector3D center;

    cb::SmartPointer<GLObject> background;
    cb::SmartPointer<GLProgram> program;

  public:
    virtual ~GLScene() {}

    unsigned getWidth() const {return width;}
    unsigned getHeight() const {return height;}
    double getAspect() const {return (double)width / height;}

    void setZoom(double zoom) {this->zoom = zoom;}
    double getZoom() const {return zoom;}

    void setRotation(const cb::QuaternionD &rotation)
      {this->rotation = rotation;}
    const cb::QuaternionD &getRotation() const {return rotation;}

    void setTranslation(const cb::Vector2D &translation)
      {this->translation = translation;}
    const cb::Vector2D &getTranslation() const {return translation;}

    void setViewBounds(const cb::Rectangle3D &bbox) {this->bbox = bbox;}
    void setViewCenter(const cb::Vector3D &center) {this->center = center;}

    void setBackground(const cb::SmartPointer<GLObject> &background)
      {this->background = background;}

    GLProgram &getProgram() {return *program;}

    virtual void glResize(unsigned width, unsigned height);
    virtual void glInit();
    virtual void glDraw();
    using GLComposite::glDraw;
  };
}
