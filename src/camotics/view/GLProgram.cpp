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
#include "GL.h"

#include <cbang/Exception.h>
#include <cbang/Catch.h>

#include <vector>

using namespace CAMotics;
using namespace std;


GLProgram::GLProgram() {
  program = getGLFuncs().glCreateProgram();
  if (!program) THROW("Failed to create GL program");
}


GLProgram::~GLProgram() {
  try {
  //if (program) getGLFuncs().glDeleteProgram(program); TODO
  } CATCH_ERROR;
}


void GLProgram::attach(const GLShader &shader) {
  getGLFuncs().glAttachShader(program, shader.get());

  GLenum err = getGLFuncs().glGetError();

  if (err == GL_INVALID_VALUE) THROW("Invalid GL program or shader");
  if (err == GL_INVALID_OPERATION)
    THROW("Invalid GL program, shader or shader already attached");
}


void GLProgram::attach(const string &resource, int type) {
  attach(GLShader::loadResource(resource, type));
}


void GLProgram::bindAttribute(const string &name, unsigned id) {
  getGLFuncs().glBindAttribLocation(program, id, (GLchar *)name.c_str());
}


void GLProgram::link() {
  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glLinkProgram(program);

  GLint success = 0;
  glFuncs.glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    GLint len = 0;
    glFuncs.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    // The len includes the NULL character
    vector<GLchar> error(len);
    glFuncs.glGetProgramInfoLog(program, len, &len, &error[0]);

    THROW("Failed to link GL program: " << &error[0]);
  }

  // Detatch all shaders
  GLint count;
  glFuncs.glGetProgramiv(program, GL_ATTACHED_SHADERS, &count);

  vector<GLuint> shaders(count);
  glFuncs.glGetAttachedShaders(program, count, 0, &shaders[0]);

  for (unsigned i = 0; i < shaders.size(); i++)
    glFuncs.glDetachShader(program, shaders[i]);
}


void GLProgram::use() const {
  getGLFuncs().glUseProgram(program);
  logGLErrors();
}


GLUniform GLProgram::getUniform(const string &name) const {
  GLint loc = glGetUniformLocation(program, (GLchar *)name.c_str());
  if (loc == -1) THROW("GL uniform '" << name << "' not found");

  return GLUniform(program, (unsigned)loc);
}
