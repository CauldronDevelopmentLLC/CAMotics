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

#include "GLProgram.h"
#include "GLShader.h"
#include "GLContext.h"

#include <cbang/Exception.h>
#include <cbang/Catch.h>

#include <vector>

using namespace CAMotics;
using namespace cb;
using namespace std;


GLProgram::GLProgram() {
  program = GLContext().glCreateProgram();
  if (!program) THROW("Failed to create GL program");
}


GLProgram::~GLProgram() {
  try {
    if (program && GLContext::isActive()) GLContext().glDeleteProgram(program);
  } CATCH_ERROR;
}


bool GLProgram::inUse() const {
  GLint current = 0;
  GLContext().glGetIntegerv(GL_CURRENT_PROGRAM, &current);
  return program == (unsigned)current;
}


void GLProgram::attach(const GLShader &shader) {
  GLContext gl;
  gl.glAttachShader(program, shader.get());

  GLenum err = gl.glGetError();

  if (err == GL_INVALID_VALUE) THROW("Invalid GL program or shader");
  if (err == GL_INVALID_OPERATION)
    THROW("Invalid GL program, shader or shader already attached");
}


void GLProgram::attach(const string &resource, int type) {
  attach(GLShader::loadResource(resource, type));
}


void GLProgram::bindAttribute(const string &name, unsigned id) {
  GLContext().glBindAttribLocation(program, id, (GLchar *)name.c_str());
}


void GLProgram::link() {
  GLContext gl;

  gl.glLinkProgram(program);

  GLint success = 0;
  gl.glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    GLint len = 0;
    gl.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    // The len includes the NULL character
    vector<GLchar> error(len);
    gl.glGetProgramInfoLog(program, len, &len, &error[0]);

    THROW("Failed to link GL program: " << &error[0]);
  }

  // Detach all shaders
  GLint count;
  gl.glGetProgramiv(program, GL_ATTACHED_SHADERS, &count);

  vector<GLuint> shaders(count);
  gl.glGetAttachedShaders(program, count, 0, &shaders[0]);

  for (unsigned i = 0; i < shaders.size(); i++)
    gl.glDetachShader(program, shaders[i]);
}


void GLProgram::use() const {
  if (inUse()) return;
  GLContext gl;
  gl.glUseProgram(program);
  gl.logErrors();
}


void GLProgram::unuse() const {GLContext().glUseProgram(0);}


unsigned GLProgram::getUniform(const string &name) {
  auto it = uniforms.find(name);
  if (it != uniforms.end()) return it->second;

  GLchar *n = (GLchar *)name.c_str();
  GLint loc = GLContext().glGetUniformLocation(program, n);
  if (loc == -1) THROW("GL uniform '" << name << "' not found");

  return uniforms[name] = (unsigned)loc;
}


void GLProgram::set(const string &name, double v0) {
  GLContext().glUniform1f(getUniform(name), v0);
}


void GLProgram::set(const string &name, double v0, double v1) {
  GLContext().glUniform2f(getUniform(name), v0, v1);
}



void GLProgram::set(const string &name, double v0, double v1, double v2) {
  GLContext().glUniform3f(getUniform(name), v0, v1, v2);
}



void GLProgram::set
(const string &name, double v0, double v1, double v2, double v3) {
  GLContext().glUniform4f(getUniform(name), v0, v1, v2, v3);
}


void GLProgram::set(const string &name, const Vector3F &v) {
  set(name, v[0], v[1], v[2]);
}


void GLProgram::set(const string &name, const Vector4F &v) {
  set(name, v[0], v[1], v[2], v[3]);
}


void GLProgram::set(const string &name, const Vector3D &v) {
  set(name, v[0], v[1], v[2]);
}


void GLProgram::set(const string &name, const Vector4D &v) {
  set(name, v[0], v[1], v[2], v[3]);
}


void GLProgram::set(const string &name, int v0) {
  GLContext().glUniform1i(getUniform(name), v0);
}



void GLProgram::set(const string &name, int v0, int v1) {
  GLContext().glUniform2i(getUniform(name), v0, v1);
}



void GLProgram::set(const string &name, int v0, int v1, int v2) {
  GLContext().glUniform3i(getUniform(name), v0, v1, v2);
}



void GLProgram::set(const string &name, int v0, int v1, int v2, int v3) {
  GLContext().glUniform4i(getUniform(name), v0, v1, v2, v3);
}


void GLProgram::set(const string &name, const Matrix4x4F &m) {
  float v[16];

  for (unsigned col = 0; col < 4; col++)
    for (unsigned row = 0; row < 4; row++)
      v[col * 4 + row] = m[row][col];

  GLContext().glUniformMatrix4fv(getUniform(name), 1, false, v);
}


void GLProgram::set(const string &name, const Matrix4x4D &m) {
  float v[16];

  for (unsigned col = 0; col < 4; col++)
    for (unsigned row = 0; row < 4; row++)
      v[col * 4 + row] = m[row][col];

  GLContext().glUniformMatrix4fv(getUniform(name), 1, false, v);
}


void GLProgram::set(const string &name, const Matrix3x3F &m) {
  float v[9];

  for (unsigned col = 0; col < 3; col++)
    for (unsigned row = 0; row < 3; row++)
      v[col * 3 + row] = m[row][col];

  GLContext().glUniformMatrix3fv(getUniform(name), 1, false, v);
}


void GLProgram::set(const string &name, const Matrix3x3D &m) {
  float v[9];

  for (unsigned col = 0; col < 3; col++)
    for (unsigned row = 0; row < 3; row++)
      v[col * 3 + row] = m[row][col];

  GLContext().glUniformMatrix3fv(getUniform(name), 1, false, v);
}
