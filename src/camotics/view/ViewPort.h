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

#include "GLProgram.h"
#include "GLUniform.h"

#include <cbang/geom/Quaternion.h>
#include <cbang/geom/Rectangle.h>


namespace CAMotics {
  class ViewPort {
    unsigned width;
    unsigned height;
    double zoom;

    cb::Vector3D rotationStartVec;
    cb::QuaternionD rotationStart;
    cb::QuaternionD rotation;

    cb::Vector2D translationStart;
    cb::Vector2D translationStartPt;
    cb::Vector2D translation;

    cb::SmartPointer<GLProgram> glProgram;
    GLUniform mvp;

  public:
    ViewPort();
    virtual ~ViewPort() {}

    unsigned getWidth() const {return width;}
    unsigned getHeight() const {return height;}
    double getZoom() const {return zoom;}
    const cb::QuaternionD &getRotation() const {return rotation;}

    void zoomIn();
    void zoomOut();

    void center();
    void resize(unsigned width, unsigned height);

    cb::Vector3D findBallVector(const cb::Vector2U &p) const;
    cb::Vector3D findBallVector(int x, int y) const;

    void startRotation(int x, int y);
    void updateRotation(int x, int y);

    void startTranslation(int x, int y);
    void updateTranslation(int x, int y);

    void resetView(char c = 'p');

    virtual void glInit();
    virtual void glDraw(const cb::Rectangle3D &bounds,
                        const cb::Vector3D &center) const;

    void drawAxes(const cb::Rectangle3D &bounds) const;
    void drawAxis(int axis, bool up, double length, double radius) const;
    void setLighting(bool x) const;
    void setWire(bool x) const;
  };
}
