/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "GL.h"
#include "BoundsView.h"

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


ViewPort::ViewPort() :
  width(1024), height(768), zoom(1), axes(true), bounds(true) {
  resetView();
}


void ViewPort::zoomIn() {
  if (0.2 < zoom) zoom *= 0.9;
}


void ViewPort::zoomOut() {
  if (zoom < 30) zoom *= 1.1;
}


void ViewPort::center() {
  translation = Vector2D(0, 0);
  zoom = 0.8;
}


void ViewPort::resize(unsigned width, unsigned height) {
  if (!height) height = 1; // Avoid div by 0

  this->width = width;
  this->height = height;

  glViewport(0, 0, width, height);
}


Vector3D ViewPort::findBallVector(const Vector2U &p) const {
  return findBallVector(p.x(), p.y());
}


Vector3D ViewPort::findBallVector(int px, int py) const {
  double x = (double)px / (width / 2.0) - 1.0;
  double y = 1.0 - (double)py / (height / 2.0);

  // Scale one dim
  if (width < height) x *= (double)width / height;
  else y *= (double)height / width;

  double z2 = 1.0 - x * x - y * y;
  double z = 0 < z2 ? sqrt(z2) : 0; // Clamp to 0
  
  return Vector3D(x, y, z).normalize();
}


void ViewPort::startRotation(int x, int y) {
  rotationStartVec = findBallVector(x, y);
  rotationStart = rotationQuat;
}


void ViewPort::updateRotation(int x, int y) {
  Vector3D current = findBallVector(x, y);
  double angle = fmod((4 * rotationStartVec.angleBetween(current)), (2 * M_PI));
  QuaternionD delta(AxisAngleD(angle, rotationStartVec.crossProduct(current)));
  rotationQuat =
    QuaternionD(delta.normalize()).multiply(rotationStart).normalize();

  rotationQuat.toAxisAngle().toGLRotation(rotation);
}


void ViewPort::startTranslation(int x, int y) {
  translationStartPt = Vector2D(x, -y);
  translationStart = translation;
}


void ViewPort::updateTranslation(int x, int y) {
  translation = translationStart + (Vector2D(x, -y) - translationStartPt)
    / Vector2D(width, height) * 2;
}


void ViewPort::resetView(char c) {
  switch (c) {
  case 'p': rotationQuat = QuaternionD(-0.45, -0.25, -0.5, 0.71); break;
  case 't': rotationQuat = QuaternionD(0, 0, 0, 0.9999); break;
  case 'b': rotationQuat = QuaternionD(0, 1, 0, 0.0001); break;
  case 'F': rotationQuat = QuaternionD(-0.71, 0, 0, 0.71); break;
  case 'B': rotationQuat = QuaternionD(0, -0.71, -0.71, 0.0001); break;
  case 'l': rotationQuat = QuaternionD(-0.5, 0.5, 0.5, 0.5); break;
  case 'r': rotationQuat = QuaternionD(-0.5, -0.5, -0.5, 0.5); break;
  }

  rotationQuat.toAxisAngle().toGLRotation(rotation);

  center();
}


void ViewPort::glInit() const {
  static const float ambient[]          = {0.50, 0.50, 0.50, 1.00};
  static const float diffuse[]          = {0.75, 0.75, 0.75, 1.00};
  static const float specular[]         = {1.00, 1.00, 1.00, 1.00};
  static const float materialSpecular[] = {0.25, 0.25, 0.25, 0.70};

  glColor4f(1, 1, 1, 1);
  glClearColor(0, 0, 0, 0);
  glClearDepth(1);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LINE_STIPPLE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glEnable(GL_LIGHT0);

  glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
  glMaterialf(GL_FRONT, GL_SHININESS, 25);
}


void ViewPort::glDraw(const Rectangle3R &bbox) const {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Background
  glBegin(GL_QUADS);
  glColor3ub(0x25, 0x30, 0x40);
  glVertex2f(-1, -1);
  glVertex2f(1, -1);
  glColor3ub(0x5, 0x5, 0x5);
  glVertex2f(1, 1);
  glVertex2f(-1, 1);
  glEnd();

  // Compute "radius"
  Vector3R dims = bbox.getDimensions();
  real radius = dims.x() < dims.y() ? dims.y() : dims.x();
  radius = dims.z() < radius ? radius : dims.z();

  // Perspective
  gluPerspective(45, (float)width / height, 1, 100000);

  // Translate
  glTranslatef(translation.x() * radius / zoom,
               translation.y() * radius / zoom, 0);

  // Scale
  gluLookAt(0, 0, radius / zoom, 0, 0, 0, 0, 1, 0);

  glMatrixMode(GL_MODELVIEW);

  // Rotate
  glRotated(rotation[0], rotation[1], rotation[2], rotation[3]);

  // Center
  Vector3R center = bbox.getCenter();
  glTranslatef(-center.x(), -center.y(), -center.z());

  // Axes
  if (axes) {
    double length = (bbox.getWidth() + bbox.getLength() + bbox.getHeight()) / 3;
    length *= 0.1;
    double radius = length / 20;

    setLighting(true);

    for (int axis = 0; axis < 3; axis++)
      for (int up = 0; up < 2; up++)
        drawAxis(axis, up, length, radius);

    setLighting(false);
  }
  
  // Bounds
  if (bounds) {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x5555);
    glLineWidth(1);
    glColor4f(1, 1, 1, 0.5); // White

    BoundsView(bbox).draw();

    glDisable(GL_LINE_STIPPLE);
  }
}


void ViewPort::drawAxis(int axis, bool up, double length, double radius) const {
  GLUquadric *quad = gluNewQuadric();
  glPushMatrix();

  switch (axis) {
  case 0:
    glColor4f(1, 0, 0, 1); // Red
    glRotated(up ? 90 : 270, 0, 1, 0);
    break;

  case 1:
    glColor4f(0, 0.75, 0.1, 1); // Green
    glRotated(up ? 270 : 90, 1, 0, 0);
    break;

  case 2:
    glColor4f(0, 0, 1, 1); // Blue
    glRotated(up ? 0 : 180, 1, 0, 0);
    break;
  }

  // Shaft
  gluCylinder(quad, radius, radius, length, 100, 100);

  // Head
  glTranslated(0, 0, length);
  gluDisk(quad, 0, 1.5 * radius, 100, 100);
  gluCylinder(quad, 1.5 * radius, 0, 2 * radius, 100, 100);

  glPopMatrix();
  gluDeleteQuadric(quad);
}


void ViewPort::setLighting(bool x) const {
  if (x) {
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

  } else {
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
  }
}


void ViewPort::setWire(bool x) const {
  if (x) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1);

  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LINE_SMOOTH);
  }
}
