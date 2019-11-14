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

#include <QOpenGLExtraFunctions>
#include <QtGui/qopenglext.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


namespace {
  Matrix4x4F perspective(float fovy, float aspect, float zNear, float zFar) {
    float a = fovy * M_PI / 180 / 2;
    float f = cos(a) / sin(a);
    Matrix4x4F m;

    m[0][0] = f / aspect;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;

    m[1][0] = 0;
    m[1][1] = f;
    m[1][2] = 0;
    m[1][3] = 0;

    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = (zFar + zNear) / (zNear - zFar);
    m[2][3] = 2 * zFar * zNear / (zNear - zFar);

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = -1;
    m[3][3] = 0;

    return m;
  }


  Matrix4x4F translate(const Vector3F &v) {
    Matrix4x4F m;

    m.toIdentity();
    m[0][3] = v[0];
    m[1][3] = v[1];
    m[2][3] = v[2];

    return m;
  }


  Matrix4x4F rotate(const QuaternionD &q) {
    Matrix4x4F m;
    AxisAngleD a = q.toAxisAngle();
    Vector3D v = a.getVector().normalize();
    double x = v.x();
    double y = v.y();
    double z = v.z();
    double c = cos(a.angle());
    double s = sin(a.angle());

    m.toIdentity();
    if (!v.isReal()) return m;

    m[0][0] = x * x * (1 - c) + c;
    m[0][1] = x * y * (1 - c) - x * s;
    m[0][2] = x * z * (1 - c) + y * s;
    m[0][3] = 0;

    m[1][0] = y * x * (1 - c) + z * s;
    m[1][1] = y * y * (1 - c) + c;
    m[1][2] = y * z * (1 - c) - x * s;
    m[1][3] = 0;

    m[2][0] = x * z * (1 - c) - y * s;
    m[2][1] = y * z * (1 - c) + x * s;
    m[2][2] = z * z * (1 - c) + c;
    m[2][3] = 0;

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;

    return m;
  }


  Matrix4x4F lookAt(const Vector3F &eye, const Vector3F &center,
                    const Vector3F &up) {
    Vector3F f = (center - eye).normalize();
    Vector3F s = f.cross(up.normalize()).normalize();
    Vector3F u = s.normalize().cross(f).normalize();

    Matrix4x4F m;
    m[0][0] = s[0];
    m[0][1] = s[1];
    m[0][2] = s[2];
    m[0][3] = 0;

    m[1][0] = u[0];
    m[1][1] = u[1];
    m[1][2] = u[2];
    m[1][3] = 0;

    m[2][0] = -f[0];
    m[2][1] = -f[1];
    m[2][2] = -f[2];
    m[2][3] = 0;

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;

    // Translate eye to origin
    return m * translate(-eye);
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
  rotationStart = rotation;
}


void ViewPort::updateRotation(int x, int y) {
  Vector3D current = findBallVector(x, y);
  double angle = fmod((4 * rotationStartVec.angleBetween(current)), (2 * M_PI));
  QuaternionD delta(AxisAngleD(angle, rotationStartVec.crossProduct(current)));
  QuaternionD newQuat =
    QuaternionD(delta.normalize()).multiply(rotationStart).normalize();

  if (!newQuat.isReal()) return;

  rotation = newQuat;
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
  case 'p': rotation = QuaternionD(-0.4,   0,     0,    0.9);   break;
  case 't': rotation = QuaternionD( 0,     0,     0,    0.999); break;
  case 'b': rotation = QuaternionD( 0,     1,     0,    0.001); break;
  case 'F': rotation = QuaternionD(-0.71,  0,     0,    0.71);  break;
  case 'B': rotation = QuaternionD( 0,    -0.71, -0.71, 0.001); break;
  case 'l': rotation = QuaternionD(-0.5,   0.5,   0.5,  0.5);   break;
  case 'r': rotation = QuaternionD(-0.5,  -0.5,  -0.5,  0.5);   break;
  }

  center();
}


void ViewPort::glInit() {
  glProgram = new GLProgram;
  glProgram->attach("shaders/tool_path_vert.glsl", GL_VERTEX_SHADER);
  glProgram->attach("shaders/tool_path_frag.glsl", GL_FRAGMENT_SHADER);
  glProgram->bindAttribute("position", GL_ATTR_POSITION);
  glProgram->bindAttribute("color",    GL_ATTR_COLOR);
  glProgram->link();
  mvp = glProgram->getUniform("mvp");

  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glClearColor(0, 0, 0, 0);

  glFuncs.glEnable(GL_LINE_SMOOTH);
  glFuncs.glEnable(GL_POINT_SMOOTH);
  glFuncs.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glFuncs.glEnable(GL_BLEND);
}


void ViewPort::glDraw(const Rectangle3D &bbox, const Vector3D &center) const {
  glProgram->use();

  GLFuncs &glFuncs = getGLFuncs();

  Matrix4x4F m;
  m.toIdentity();
  mvp.set(m);

  // Background
  float colors[] = {
    0.15, 0.19, 0.25,
    0.15, 0.19, 0.25,
    0.02, 0.02, 0.02,

    0.15, 0.19, 0.25,
    0.02, 0.02, 0.02,
    0.02, 0.02, 0.02,
  };
  const float v[] = {
    -1, -1, 0, 1, -1, 0, -1, 1, 0,
     1, -1, 0, 1,  1, 0, -1, 1, 0,
  };

  glFuncs.glEnableVertexAttribArray(GL_ATTR_POSITION);
  glFuncs.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, v);

  glFuncs.glEnableVertexAttribArray(GL_ATTR_COLOR);
  glFuncs.glVertexAttribPointer(GL_ATTR_COLOR, 3, GL_FLOAT, false, 0, colors);

  glFuncs.glDrawArrays(GL_TRIANGLES, 0, 6);

  glFuncs.glDisableVertexAttribArray(GL_ATTR_POSITION);
  glFuncs.glDisableVertexAttribArray(GL_ATTR_COLOR);

  // Compute "radius"
  Vector3D dims = bbox.getDimensions();
  double radius = dims.x() < dims.y() ? dims.y() : dims.x();
  radius = dims.z() < radius ? radius : dims.z();

  // Projection
  m *= perspective(45, (float)width / height, 1, 100000);

  // Translate
  m *= translate(Vector3F(translation.x(), translation.y(), 0) * radius / zoom);

  // View
  m *= lookAt(Vector3F(0, 0, radius / zoom), Vector3F(), Vector3F(0, 1, 0));

  // Rotate
  m *= rotate(rotation);

  // Center
  m *= translate(-center);

  mvp.set(m);
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
#if 0 // TODO GL
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
#endif
}


void ViewPort::setLighting(bool x) const {
#if 0 // TODO GL
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
#endif
}


void ViewPort::setWire(bool x) const {
#if 0 // TODO GL
  GLFuncs &glFuncs = getGLFuncs();

  if (x) {
    glFuncs.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glFuncs.glDisable(GL_LINE_SMOOTH);
    glFuncs.glLineWidth(1);

  } else {
    glFuncs.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFuncs.glEnable(GL_LINE_SMOOTH);
  }
#endif
}
