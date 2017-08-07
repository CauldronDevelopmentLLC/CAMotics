/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


namespace {
  void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear,
                      GLdouble zFar) {
    GLdouble ymax = zNear * tan(fovy * M_PI / 360.0);
    GLdouble ymin = -ymax;

    GLdouble xmin = ymin * aspect;
    GLdouble xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
  }


  void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx,
                 GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy,
                 GLdouble upz) {
    GLdouble m[16];
    GLdouble x[3], y[3], z[3];
    GLdouble mag;

    // Make rotation matrix

    // Z vector
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) { // mpichler, 19950515
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
    }

    // Y vector
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    // X vector = Y cross Z
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    // Recompute Y = Z cross X
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    // mpichler, 19950515
    // cross product gives area of parallelogram, which is < 1.0 for
    // non-perpendicular unit-length vectors; so normalize x, y here

    mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
    }

    mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
    }

#define M(row, col) m[col * 4 + row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    glMultMatrixd(m);

    // Translate Eye to Origin
    glTranslated(-eyex, -eyey, -eyez);

  }
}

ViewPort::ViewPort() : width(1024), height(768), zoom(1) {resetView();}


void ViewPort::zoomIn() {if (0.2 < zoom) zoom *= 0.9;}
void ViewPort::zoomOut() {if (zoom < 30) zoom *= 1.1;}


void ViewPort::center() {
  translation = cb::Vector2D(0, 0);
  zoom = 0.8;
}


void ViewPort::resize(unsigned width, unsigned height) {
  if (!height) height = 1; // Avoid div by 0

  this->width = width;
  this->height = height;

  glViewport(0, 0, width, height);
}


cb::Vector3D ViewPort::findBallVector(const cb::Vector2U &p) const {
  return findBallVector(p.x(), p.y());
}


cb::Vector3D ViewPort::findBallVector(int px, int py) const {
  double x = (double)px / (width / 2.0) - 1.0;
  double y = 1.0 - (double)py / (height / 2.0);

  // Scale one dim
  if (width < height) x *= (double)width / height;
  else y *= (double)height / width;

  double z2 = 1.0 - x * x - y * y;
  double z = 0 < z2 ? sqrt(z2) : 0; // Clamp to 0

  return cb::Vector3D(x, y, z).normalize();
}


void ViewPort::startRotation(int x, int y) {
  rotationStartVec = findBallVector(x, y);
  rotationStart = rotationQuat;
}


void ViewPort::updateRotation(int x, int y) {
  cb::Vector3D current = findBallVector(x, y);
  double angle = fmod((4 * rotationStartVec.angleBetween(current)), (2 * M_PI));
  QuaternionD delta(AxisAngleD(angle, rotationStartVec.crossProduct(current)));
  rotationQuat =
    QuaternionD(delta.normalize()).multiply(rotationStart).normalize();

  if (!rotationQuat.isReal()) resetView();

  rotationQuat.toAxisAngle().toGLRotation(rotation);
}


void ViewPort::startTranslation(int x, int y) {
  translationStartPt = cb::Vector2D(x, -y);
  translationStart = translation;
}


void ViewPort::updateTranslation(int x, int y) {
  translation = translationStart + (cb::Vector2D(x, -y) - translationStartPt)
    / cb::Vector2D(width, height) * 2;
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


void ViewPort::glDraw(const cb::Rectangle3D &bbox,
                      const cb::Vector3D &center) const {
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
  cb::Vector3D dims = bbox.getDimensions();
  double radius = dims.x() < dims.y() ? dims.y() : dims.x();
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
  glTranslatef(-center.x(), -center.y(), -center.z());
}


void ViewPort::drawAxes(const Rectangle3D &bbox) const {
  double length = (bbox.getWidth() + bbox.getLength() + bbox.getHeight()) / 3;
  length *= 0.1;
  double radius = length / 20;

  for (int axis = 0; axis < 3; axis++)
    for (int up = 0; up < 2; up++)
      drawAxis(axis, up, length, radius);
}


void ViewPort::drawAxis(int axis, bool up, double length, double radius) const {
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
  glCylinder(radius, radius, length, 128);

  // Head
  glTranslated(0, 0, length);
  glDisk(1.5 * radius, 128);
  glCylinder(1.5 * radius, 0, 2 * radius, 128);

  glPopMatrix();
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
