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

#include "GL.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>
#include <cbang/Math.h>

#include <QOpenGLFunctions>

using namespace std;
using namespace cb;
using namespace CAMotics;


namespace CAMotics {
  const char *glErrorString(GLenum err) {
    switch (err) {
    case GL_NO_ERROR: return "No error";
    case GL_INVALID_ENUM: return "Invalid enum";
    case GL_INVALID_VALUE: return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer op";
    case GL_OUT_OF_MEMORY: return "Out of memory";
    }
    return "Unknown error";
  }


  void logGLErrors() {
    GLenum err;
    do {
      err = getGLFuncs().glGetError();
      if (err == GL_NO_ERROR) break;
      LOG_ERROR("GL error: " << err << ": " << glErrorString(err));
    } while (err != GL_CONTEXT_LOST);
  }


  QOpenGLContext &getGLCtx() {
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) THROW("No active OpenGL context");
    return *ctx;
  }


  GLFuncs &getGLFuncs() {
    GLFuncs *glFuncs = getGLCtx().functions();
    if (!glFuncs) THROW("Failed to access OpenGL functions");
    return *glFuncs;
  }


  void glDisk(double radius, unsigned segments) {
#if 0 // TODO GL
    GLFuncs &glFuncs = getGLFuncs();

    unsigned count = 1 + segments;
    SmartPointer<double>::Array v = new double[count * 2];

    v[0] = v[1] = 0;

    for (unsigned i = 0; i < segments; i++) {
      float a = i * 2 * M_PI / (segments - 1);
      v[(i + 1) * 2 + 0] = sin(a) * radius;
      v[(i + 1) * 2 + 1] = cos(a) * radius;
    }

    glFuncs.glVertexPointer(2, GL_DOUBLE, 0, v.get());
    glFuncs.glEnableClientState(GL_VERTEX_ARRAY);
    glFuncs.glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glFuncs.glDisableClientState(GL_VERTEX_ARRAY);
#endif
  }


  void glCylinder(double base, double top, double height, unsigned segments) {
#if 0 // TODO GL
    GLFuncs &glFuncs = getGLFuncs();

    unsigned count = segments * 2;
    SmartPointer<double>::Array v = new double[count * 3];
    SmartPointer<double>::Array n = new double [count * 3];

    double b = sqrt((base - top) * (base - top) + height * height);

    for (unsigned i = 0; i < segments; i++) {
      unsigned o = i * 3 * 2;
      float a = i * 2 * M_PI / (segments - 1);

      // Normals
      n[o + 0] = n[o + 3] = cos(a) * height / b;
      n[o + 1] = n[o + 4] = sin(a) * height / b;
      n[o + 2] = n[o + 5] = (base - top) / b;

      // Vertices
      v[o + 0] = base * cos(a);
      v[o + 1] = base * sin(a);
      v[o + 2] = 0;
      v[o + 3] = top * cos(a);
      v[o + 4] = top * sin(a);
      v[o + 5] = height;
    }

    glFuncs.glNormalPointer(GL_DOUBLE, 0, n.get());
    glFuncs.glVertexPointer(3, GL_DOUBLE, 0, v.get());
    glFuncs.glEnableClientState(GL_NORMAL_ARRAY);
    glFuncs.glEnableClientState(GL_VERTEX_ARRAY);
    glFuncs.glDrawArrays(GL_QUAD_STRIP, 0, count);
    glFuncs.glDisableClientState(GL_NORMAL_ARRAY);
    glFuncs.glDisableClientState(GL_VERTEX_ARRAY);
#endif
  }


  void glConic(double radiusA, double radiusB, double length,
               unsigned segments) {
#if 0 // TODO GL
    GLFuncs &glFuncs = getGLFuncs();

    // Body
    glCylinder(radiusA, radiusB, length, segments);

    // End caps
    if (radiusA) {
      glFuncs.glNormal3f(0, 0, -1);
      glDisk(radiusA, segments);
    }

    if (radiusB) {
      glFuncs.glPushMatrix();
      glFuncs.glTranslatef(0, 0, length);
      glFuncs.glNormal3f(0, 0, 1);
      glDisk(radiusB, segments);
      glFuncs.glPopMatrix();
    }
#endif
  }


  void glSphere(double radius, unsigned lats, unsigned lngs) {
#if 0 // TODO GL
    GLFuncs &glFuncs = getGLFuncs();

    unsigned count = lngs * 2;
    SmartPointer<double>::Array n = new double[count * 3];
    SmartPointer<double>::Array v = new double[count * 3];

    glFuncs.glNormalPointer(GL_DOUBLE, 0, n.get());
    glFuncs.glVertexPointer(3, GL_DOUBLE, 0, v.get());
    glFuncs.glEnableClientState(GL_NORMAL_ARRAY);
    glFuncs.glEnableClientState(GL_VERTEX_ARRAY);

    for (unsigned i = 0; i < lats; i++) {
      double lat0 = M_PI * (-0.5 + (double)(i - 1) / (lats - 1));
      double z0 = sin(lat0);
      double zr0 = cos(lat0);

      double lat1 = M_PI * (-0.5 + (double)i / (lats - 1));
      double z1 = sin(lat1);
      double zr1 = cos(lat1);

      for (unsigned j = 0; j < lngs; j++) {
        double lng = 2 * M_PI * (double)(j - 1) / (lngs - 1);
        double x = cos(lng);
        double y = sin(lng);
        unsigned o = j * 3 * 2;

        n[o + 0] = x * zr0; n[o + 1] = y * zr0; n[o + 2] = z0;
        n[o + 3] = x * zr1; n[o + 4] = y * zr1; n[o + 5] = z1;

        v[o + 0] = x * zr0 * radius;
        v[o + 1] = y * zr0 * radius;
        v[o + 2] = z0 * radius;
        v[o + 3] = x * zr1 * radius;
        v[o + 4] = y * zr1 * radius;
        v[o + 5] = z1 * radius;
      }

      glFuncs.glDrawArrays(GL_QUAD_STRIP, 0, count);
    }

    glFuncs.glDisableClientState(GL_NORMAL_ARRAY);
    glFuncs.glDisableClientState(GL_VERTEX_ARRAY);
#endif
  }
}
