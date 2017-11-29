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


  void checkGLError(const string &message) {
    GLenum err = getGLFuncs().glGetError();
    if (err != GL_NO_ERROR)
      LOG_ERROR(message << "GL error: " << err << ": " << glErrorString(err));
  }


  bool haveVBOs() {
    if (!QOpenGLContext::currentContext()) return false;
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    return glFuncs && glFuncs->hasOpenGLFeature(QOpenGLFunctions::Buffers);
  }


  void glDisk(double radius, unsigned segments) {
    GLFuncs &glFuncs = getGLFuncs();

    glFuncs.glBegin(GL_TRIANGLE_FAN);

    glFuncs.glVertex2f(0, 0);

    for (unsigned i = 0; i < segments; i++) {
      float a = i * 2 * M_PI / (segments - 1);
      glFuncs.glVertex2f(sin(a) * radius, cos(a) * radius);
    }

    glFuncs.glEnd();
  }


  void glCylinder(double base, double top, double height, unsigned segments) {
    double b = sqrt((base - top) * (base - top) + height * height);

    GLFuncs &glFuncs = getGLFuncs();
    glFuncs.glBegin(GL_QUAD_STRIP);

    for (unsigned i = 0; i < segments; i++) {
      float a = i * 2 * M_PI / (segments - 1);

      glFuncs.glNormal3f(cos(a) * height / b, sin(a) * height / b,
                         (base - top) / b);
      glFuncs.glVertex3f(base * cos(a), base * sin(a), 0);
      glFuncs.glVertex3f(top * cos(a), top * sin(a), height);
    }

    glFuncs.glEnd();
  }


  void glSphere(double radius, unsigned lats, unsigned lngs) {
    GLFuncs &glFuncs = getGLFuncs();

    for (unsigned i = 0; i < lats; i++) {
      double lat0 = M_PI * (-0.5 + (double)(i - 1) / (lats - 1));
      double z0 = sin(lat0);
      double zr0 = cos(lat0);

      double lat1 = M_PI * (-0.5 + (double)i / (lats - 1));
      double z1 = sin(lat1);
      double zr1 = cos(lat1);

      glFuncs.glBegin(GL_QUAD_STRIP);

      for (unsigned j = 0; j < lngs; j++) {
        double lng = 2 * M_PI * (double)(j - 1) / (lngs - 1);
        double x = cos(lng);
        double y = sin(lng);

        glFuncs.glNormal3f(x * zr0, y * zr0, z0);
        glFuncs.glVertex3f(x * zr0 * radius, y * zr0 * radius, z0 * radius);
        glFuncs.glNormal3f(x * zr1, y * zr1, z1);
        glFuncs.glVertex3f(x * zr1 * radius, y * zr1 * radius, z1 * radius);
      }

      glFuncs.glEnd();
    }
  }


  GLFuncs &getGLFuncs() {
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) THROW("No active OpenGL context");
    GLFuncs *glFuncs = ctx->versionFunctions<GLFuncs>();
    if (!glFuncs) THROW("Failed to access OpenGL functions");
    return *glFuncs;
  }
}
