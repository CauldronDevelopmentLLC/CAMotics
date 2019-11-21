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

#include "ViewPort.h"

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ViewPort::ViewPort() {resetView();}


void ViewPort::zoomIn() {if (0.2 < getZoom()) setZoom(getZoom() * 0.9);}
void ViewPort::zoomOut() {if (getZoom() < 30) setZoom(getZoom() * 1.1);}


void ViewPort::center() {
  setTranslation(Vector2D(0, 0));
  setZoom(0.8);
}


Vector3D ViewPort::findBallVector(const Vector2U &p) const {
  return findBallVector(p.x(), p.y());
}


Vector3D ViewPort::findBallVector(int px, int py) const {
  double x = (double)px / (getWidth() / 2.0) - 1.0;
  double y = 1.0 - (double)py / (getHeight() / 2.0);

  // Scale one dim
  if (getWidth() < getHeight()) x *= (double)getWidth() / getHeight();
  else y *= (double)getHeight() / getWidth();

  double z2 = 1.0 - x * x - y * y;
  double z = 0 < z2 ? sqrt(z2) : 0; // Clamp to 0

  return Vector3D(x, y, z).normalize();
}


void ViewPort::startRotation(int x, int y) {
  rotationStartVec = findBallVector(x, y);
  rotationStart = getRotation();
}


void ViewPort::updateRotation(int x, int y) {
  Vector3D current = findBallVector(x, y);
  double angle = fmod((4 * rotationStartVec.angleBetween(current)), (2 * M_PI));
  QuaternionD delta(AxisAngleD(angle, rotationStartVec.crossProduct(current)));
  QuaternionD newQuat =
    QuaternionD(delta.normalize()).multiply(rotationStart).normalize();

  if (!newQuat.isReal()) return;

  GLScene::setRotation(newQuat);
}


void ViewPort::startTranslation(int x, int y) {
  translationStartPt = Vector2D(x, -y);
  translationStart = getTranslation();
}


void ViewPort::updateTranslation(int x, int y) {
  setTranslation(translationStart + (Vector2D(x, -y) - translationStartPt)
                 / Vector2D(getWidth(), getHeight()));
}


void ViewPort::resetView(char c) {
  QuaternionD rotation;

  switch (c) {
  case 'p': rotation = QuaternionD(-0.4,   0,     0,    0.9);   break;
  case 't': rotation = QuaternionD( 0,     0,     0,    0.999); break;
  case 'b': rotation = QuaternionD( 0,     1,     0,    0.001); break;
  case 'F': rotation = QuaternionD(-0.71,  0,     0,    0.71);  break;
  case 'B': rotation = QuaternionD( 0,    -0.71, -0.71, 0.001); break;
  case 'l': rotation = QuaternionD(-0.5,   0.5,   0.5,  0.5);   break;
  case 'r': rotation = QuaternionD(-0.5,  -0.5,  -0.5,  0.5);   break;
  }

  GLScene::setRotation(rotation);
  center();
}
