/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_VIEW_PORT_H
#define CAMOTICS_VIEW_PORT_H


#include <camotics/Geom.h>


namespace CAMotics {
  class ViewPort {
    unsigned width;
    unsigned height;
    double zoom;
    bool axes;
    bool bounds;

    cb::Vector3D rotationStartVec;
    cb::QuaternionD rotationStart;
    cb::QuaternionD rotationQuat;
    double rotation[4];

    cb::Vector2D translationStart;
    cb::Vector2D translationStartPt;
    cb::Vector2D translation;

  public:
    ViewPort();
    virtual ~ViewPort() {}

    unsigned getWidth() const {return width;}
    unsigned getHeight() const {return height;}
    double getZoom() const {return zoom;}

    void toggleShowAxes() {axes = !axes;}
    void setShowAxes(bool x) {axes = x;}
    bool getShowAxes() const {return axes;}

    void toggleShowBounds() {bounds = !bounds;}
    void setShowBounds(bool x) {bounds = x;}
    bool getShowBounds() const {return bounds;}

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

    virtual void glInit() const;
    virtual void glDraw(const Rectangle3R &bounds) const;

    void drawAxis(int axis, bool up, double length, double radius) const;
    void setLighting(bool x) const;
    void setWire(bool x) const;
  };
}

#endif // CAMOTICS_VIEW_PORT_H

