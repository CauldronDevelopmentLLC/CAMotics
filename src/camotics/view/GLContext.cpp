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

#include "GLContext.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

using namespace CAMotics;


namespace {
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


  QOpenGLContext *getOpenGLContext() {
    auto ctx = QOpenGLContext::currentContext();
    if (!ctx) THROW("No active OpenGL context");
    return ctx;
  }
}


GLContext::GLContext(QOpenGLContext *ctx) :
  QOpenGLFunctions(ctx ? ctx : getOpenGLContext()) {
  stack.push_back(Transform());
}


void GLContext::pushMatrix() {stack.push_back(stack.back());}


void GLContext::pushMatrix(const Transform &t) {
  pushMatrix();
  applyMatrix(t);
}


void GLContext::setMatrix(const Transform &t) {
  stack.back() = t;
  modelMat.set(stack.back());
}


void GLContext::applyMatrix(const Transform &t) {
  stack.back() *= t;
  modelMat.set(stack.back());
}


void GLContext::popMatrix() {
  if (stack.size() == 1) THROW("Matrix stack underrun");
  stack.pop_back();
  modelMat.set(stack.back());
}


void GLContext::logErrors() {
  GLenum err;

  do {
    err = glGetError();
    if (err == GL_NO_ERROR) break;
    LOG_ERROR("GL error: " << err << ": " << glErrorString(err));
  } while (err != GL_CONTEXT_LOST);
}


bool GLContext::isActive() {return QOpenGLContext::currentContext();}


void GLContext::setColor(const Color &color) {
  glVertexAttrib4fv(GL_ATTR_COLOR, color.data);
}


void GLContext::setColor(float r, float g, float b, float a) {
  setColor(Color(r, g, b, a));
}
