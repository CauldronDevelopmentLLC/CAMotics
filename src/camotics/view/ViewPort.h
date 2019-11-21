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

#include "GLScene.h"


namespace CAMotics {
  class ViewPort : public GLScene {
    cb::Vector3D rotationStartVec;
    cb::QuaternionD rotationStart;

    cb::Vector2D translationStart;
    cb::Vector2D translationStartPt;

  public:
    ViewPort();
    virtual ~ViewPort() {}

    void zoomIn();
    void zoomOut();

    void center();

    cb::Vector3D findBallVector(const cb::Vector2U &p) const;
    cb::Vector3D findBallVector(int x, int y) const;

    void startRotation(int x, int y);
    void updateRotation(int x, int y);

    void startTranslation(int x, int y);
    void updateTranslation(int x, int y);

    void resetView(char c = 'p');
  };
}
