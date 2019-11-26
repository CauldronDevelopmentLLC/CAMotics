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

#include <cbang/log/Logger.h>

using namespace CAMotics;
using namespace cb;
using namespace std;


namespace {
  float toRadians(float degrees) {return degrees * M_PI / 180;}
}


void GLScene::glResize(unsigned width, unsigned height) {
  if (!height) height = 1; // Avoid div by 0

  this->width = width;
  this->height = height;

  GLContext().glViewport(0, 0, width, height);
}


void GLScene::glInit() {
  GLContext gl;

  // OpenGL config
  gl.glEnable(GL_DEPTH_TEST);
  gl.glEnable(GL_LINE_SMOOTH);
  gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.glEnable(GL_BLEND);
  gl.glLineWidth(1);

  // Shaders
  program = new GLProgram;
  program->attach("shaders/phong.vert", GL_VERTEX_SHADER);
  program->attach("shaders/phong.frag", GL_FRAGMENT_SHADER);
  program->bindAttribute("position",    GL_ATTR_POSITION);
  program->bindAttribute("normal",      GL_ATTR_NORMAL);
  program->bindAttribute("color",       GL_ATTR_COLOR);
  program->link();

  // Light
  program->use();
  program->set("light.direction", 0.0,  0.25, -1.0);
  program->set("light.ambient",   0.75, 0.75,  0.75, 1.0);
  program->set("light.diffuse",   0.75, 0.75,  0.75, 1.0);
}


void GLScene::glDraw() {
  GLContext gl(this);

  gl.glClearColor(0, 0, 0, 0);
  gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  program->use();
  program->set("light.enabled", 0);

  Transform t;
  program->set("projection", t);
  program->set("view",       t);
  program->set("model",      t);

  if (background.isSet()) background->glDraw(gl);

  // Projection
  t.perspective(toRadians(45), getAspect(), 1, 100000);
  program->set("projection", t);

  // Compute camera Z
  Vector3D dims = bbox.getDimensions();
  double maxDim = dims.x() < dims.y() ? dims.y() : dims.x();
  maxDim = dims.z() < maxDim ? maxDim : dims.z();
  double cameraZ = maxDim / zoom;
  Vector3D camera(0, 0, cameraZ);

  // View
  t.toIdentity();
  t.lookAt(camera, Vector3D(), Vector3D(0, 1, 0));
  program->set("view", t);

  // Translate
  t.toIdentity();
  t.translate(Vector3D(translation.x(), translation.y(), 0) * cameraZ);

  // Rotate
  t.rotate(rotation);

  // Center
  t.translate(-center);

  gl.pushMatrix(t);
  GLComposite::glDraw(gl);
  gl.popMatrix();
}
