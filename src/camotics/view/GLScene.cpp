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

#include "GLScene.h"

using namespace CAMotics;
using namespace cb;
using namespace std;


namespace {
  float toRadians(float degrees) {return degrees * M_PI / 180;}
}


void GLScene::glResize(GLContext &gl, unsigned width, unsigned height) {
  if (!height) height = 1; // Avoid div by 0

  this->width = width;
  this->height = height;

  gl.glViewport(0, 0, width, height);
}


void GLScene::glInit(GLContext &gl) {
  glProgram = new GLProgram;
  glProgram->attach("shaders/tool_path_vert.glsl", GL_VERTEX_SHADER);
  glProgram->attach("shaders/tool_path_frag.glsl", GL_FRAGMENT_SHADER);
  glProgram->bindAttribute("position", GL_ATTR_POSITION);
  glProgram->bindAttribute("normal",   GL_ATTR_NORMAL);
  glProgram->bindAttribute("color",    GL_ATTR_COLOR);
  glProgram->link();

  projection = glProgram->getUniform("projection");
  model      = glProgram->getUniform("model");
  view       = glProgram->getUniform("view");

  gl.glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearDepthf(1);
  glDepthFunc(GL_LEQUAL);

  gl.glEnable(GL_LINE_SMOOTH);
  gl.glEnable(GL_POINT_SMOOTH);
  gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.glEnable(GL_BLEND);

  gl.glLineWidth(1);
}


void GLScene::glDraw(GLContext &gl) {
  glProgram->use();

  Transform t;
  projection.set(t);
  view.set(t);
  model.set(t);

  if (background.isSet()) background->glDraw(gl);

  // Compute camera Z
  Vector3D dims = bbox.getDimensions();
  double maxDim = dims.x() < dims.y() ? dims.y() : dims.x();
  maxDim = dims.z() < maxDim ? maxDim : dims.z();
  double cameraZ = maxDim / zoom;

  // Projection
  t.perspective(toRadians(45), getAspect(), 1, 100000);
  projection.set(t);

  // View
  t.toIdentity();
  t.lookAt(Vector3D(0, 0, cameraZ), Vector3D(), Vector3D(0, 1, 0));

  // Rotate
  t.rotate(rotation);

  // Translate
  Vector3D trans(translation.x(), translation.y(), 0);
  t.translate(trans * cameraZ - center);

  view.set(t);

  GLComposite::glDraw(gl);
}
