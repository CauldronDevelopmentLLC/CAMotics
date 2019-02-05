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

    getGLFuncs().glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
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
    getGLFuncs().glMultMatrixd(m);

    // Translate Eye to Origin
    getGLFuncs().glTranslated(-eyex, -eyey, -eyez);
  }
}

ViewPort::ViewPort() : width(1024), height(768), zoom(1) {resetView();}


void ViewPort::zoomIn() {if (0.2 < zoom) zoom *= 0.9;}
void ViewPort::zoomOut() {if (zoom < 30) zoom *= 1.1;}


void ViewPort::center() {
  translation = Vector2D(0, 0);
  zoom = 0.8;
}


void ViewPort::resize(unsigned width, unsigned height) {
  if (!height) height = 1; // Avoid div by 0

  this->width = width;
  this->height = height;

  getGLFuncs().glViewport(0, 0, width, height);
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
  QuaternionD newQuat =
    QuaternionD(delta.normalize()).multiply(rotationStart).normalize();

  if (!newQuat.isReal()) return;

  rotationQuat = newQuat;
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
  case 'p': rotationQuat = QuaternionD(-0.4, 0, 0, 0.9); break;
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

  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glColor4f(1, 1, 1, 1);
  glFuncs.glClearColor(0, 0, 0, 0);
  glFuncs.glClearDepth(1);

  glFuncs.glEnable(GL_LINE_SMOOTH);
  glFuncs.glEnable(GL_POINT_SMOOTH);
  glFuncs.glShadeModel(GL_SMOOTH);
  glFuncs.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glFuncs.glEnable(GL_BLEND);
  glFuncs.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glFuncs.glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glFuncs.glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glFuncs.glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glFuncs.glEnable(GL_LIGHT0);

  glFuncs.glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
  glFuncs.glMaterialf(GL_FRONT, GL_SHININESS, 25);
}


void ViewPort::glDraw(const Rectangle3D &bbox,
                      const Vector3D &center) const {
  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glMatrixMode(GL_MODELVIEW);
  glFuncs.glLoadIdentity();

  glFuncs.glMatrixMode(GL_PROJECTION);
  glFuncs.glLoadIdentity();

  // Background
  const double colors[] = {
    0.15, 0.19, 0.25,
    0.15, 0.19, 0.25,
    0.02, 0.02, 0.02,
    0.02, 0.02, 0.02,
  };
  const double v[] = {-1, -1, 1, -1, 1, 1, -1, 1};

  glFuncs.glVertexPointer(2, GL_DOUBLE, 0, v);
  glFuncs.glColorPointer(3, GL_DOUBLE, 0, colors);
  glFuncs.glEnableClientState(GL_VERTEX_ARRAY);
  glFuncs.glEnableClientState(GL_COLOR_ARRAY);
  glFuncs.glDrawArrays(GL_QUADS, 0, 4);
  glFuncs.glDisableClientState(GL_COLOR_ARRAY);
  glFuncs.glDisableClientState(GL_VERTEX_ARRAY);

  // Compute "radius"
  Vector3D dims = bbox.getDimensions();
  double radius = dims.x() < dims.y() ? dims.y() : dims.x();
  radius = dims.z() < radius ? radius : dims.z();

  // Perspective
  gluPerspective(45, (float)width / height, 1, 100000);

  // Translate
  glFuncs.glTranslatef(translation.x() * radius / zoom,
                       translation.y() * radius / zoom, 0);

  // Scale
  gluLookAt(0, 0, radius / zoom, 0, 0, 0, 0, 1, 0);

  glFuncs.glMatrixMode(GL_MODELVIEW);

  // Rotate
  glFuncs.glRotated(rotation[0], rotation[1], rotation[2], rotation[3]);

  // Center
  glFuncs.glTranslatef(-center.x(), -center.y(), -center.z());
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
  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glPushMatrix();

  switch (axis) {
  case 0:
    glFuncs.glColor4f(1, 0, 0, 1); // Red
    glFuncs.glRotated(up ? 90 : 270, 0, 1, 0);
    break;

  case 1:
    glFuncs.glColor4f(0, 0.75, 0.1, 1); // Green
    glFuncs.glRotated(up ? 270 : 90, 1, 0, 0);
    break;

  case 2:
    glFuncs.glColor4f(0, 0, 1, 1); // Blue
    glFuncs.glRotated(up ? 0 : 180, 1, 0, 0);
    break;
  }

  // Shaft
  glCylinder(radius, radius, length, 128);

  // Head
  glFuncs.glTranslated(0, 0, length);
  glDisk(1.5 * radius, 128);
  glCylinder(1.5 * radius, 0, 2 * radius, 128);

  glFuncs.glPopMatrix();
}


void ViewPort::setLighting(bool x) const {
  GLFuncs &glFuncs = getGLFuncs();

  if (x) {
    glFuncs.glEnable(GL_LIGHTING);
    glFuncs.glEnable(GL_DEPTH_TEST);
    glFuncs.glEnable(GL_COLOR_MATERIAL);

  } else {
    glFuncs.glDisable(GL_COLOR_MATERIAL);
    glFuncs.glDisable(GL_LIGHTING);
    glFuncs.glDisable(GL_DEPTH_TEST);
  }
}


void ViewPort::setWire(bool x) const {
  GLFuncs &glFuncs = getGLFuncs();

  if (x) {
    glFuncs.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glFuncs.glDisable(GL_LINE_SMOOTH);
    glFuncs.glLineWidth(1);

  } else {
    glFuncs.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFuncs.glEnable(GL_LINE_SMOOTH);
  }
}
