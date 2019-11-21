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

#include "View.h"
#include "GLBox.h"
#include "ToolView.h"
#include "AxesView.h"
#include "GradientBackground.h"

#include <camotics/sim/MoveLookup.h>

#include <gcode/ToolTable.h>

#include <cbang/String.h>
#include <cbang/Math.h>
#include <cbang/Catch.h>
#include <cbang/util/HumanNumber.h>
#include <cbang/time/HumanTime.h>
#include <cbang/log/Logger.h>
#include <cbang/config/Options.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


View::View(ValueSet &valueSet) :
  values(valueSet), flags(SHOW_PATH_FLAG | SHOW_TOOL_FLAG | SHOW_SURFACE_FLAG |
                          SHOW_WORKPIECE_BOUNDS_FLAG | SHOW_AXES_FLAG),
  path(new ToolPathView(valueSet)) {

  setBackground(new GradientBackground(Color(0.15, 0.19, 0.25),
                                       Color(0.02, 0.02, 0.02)));

  values.add("play_speed", speed);
  values.add("play_direction", reverse);
  values.add("view_flags", flags);
}


View::~View() {}


void View::incSpeed() {
  if (speed < (1 << 16)) speed <<= 1;
  values.updated();
}


void View::decSpeed() {
  if (1 < speed) speed >>= 1;
  values.updated();
}


void View::setToolPath(const SmartPointer<GCode::ToolPath> &toolPath) {
  path->setPath(toolPath);
  path->setShowIntensity(isFlagSet(View::PATH_INTENSITY_FLAG));
}


void View::setWorkpiece(const Rectangle3D &bounds) {
  workpieceBounds = bounds;
}


void View::setSurface(const SmartPointer<Surface> &surface) {
  surfaceChanged = true;
  this->surface = surface;
}


void View::setMoveLookup(const SmartPointer<MoveLookup> &moveLookup) {
  this->moveLookup = moveLookup;
}


bool View::update() {
  // Animate
  if (isFlagSet(PLAY_FLAG)) {
    double now = Timer::now();
    double delta = (now - lastTime) * speed;

    if (!reverse && path->atEnd()) path->setByRatio(0);
    if (reverse && path->atStart()) path->setByRatio(1);

    if (lastTime && delta) {
      if (reverse) path->decTime(delta);
      else path->incTime(delta);

      if ((reverse && path->atStart()) || (!reverse && path->atEnd())) {
        if (path->atEnd()) path->setByRatio(1);
        else path->setByRatio(0);

        clearFlag(PLAY_FLAG);
      }
    }

    lastTime = now;
    return true;

  } else lastTime = 0;

  return false;
}


void View::clear() {
  setToolPath(0);
  setWorkpiece(Rectangle3D());
  surface.release();
  surfaceChanged = true;
  setMoveLookup(0);
  path->setByRatio(1);
  resetView();
}


void View::updateVisibility() {
  axes->setVisible(isFlagSet(View::SHOW_AXES_FLAG));
  bounds->setVisible(!isFlagSet(View::SHOW_WORKPIECE_FLAG) &&
                     isFlagSet(View::SHOW_WORKPIECE_BOUNDS_FLAG));
  tool->setVisible(isFlagSet(View::SHOW_TOOL_FLAG) && !path->isEmpty());
  path->setShowIntensity(isFlagSet(View::PATH_INTENSITY_FLAG));
  path->setVisible(isFlagSet(View::SHOW_PATH_FLAG));
  mesh->setVisible(isFlagSet(View::SHOW_SURFACE_FLAG));
  workpiece->setVisible(isFlagSet(View::SHOW_WORKPIECE_FLAG));
}


void View::updateBounds() {
  // Compute bounds
  Rectangle3D bbox = path->getBounds();
  if (surface.isSet()) bbox.add(surface->getBounds());
  bbox.add(workpieceBounds);
  //if (showMachine) bbox.add(machine->getBounds());

  workpiece->setBounds(workpieceBounds);
  bounds->setBounds(workpieceBounds);

  // Axes
  double length = workpieceBounds.getDimensions().sum() / 30;
  axes->getTransform().toIdentity();
  axes->getTransform().scale(length, length, length);

  // Setup view
  GLScene::setViewBounds(bbox);
  GLScene::setViewCenter(workpieceBounds.getCenter());
}


void View::glInit(GLContext &gl) {
  GLScene::glInit(gl);

  add(axes = new AxesView);
  add(bounds = new GLBox);
  bounds->setColor(1, 1, 1, 0.5); // White
  add(workpiece = new CuboidView);
  add(tool = new ToolView);
  add(path);
  add(mesh = new Mesh(0));
}


void View::glDraw(GLContext &gl) {
  updateVisibility();
  updateBounds();

  // Machine
  bool showMachine = !machine.isNull() && isFlagSet(View::SHOW_MACHINE_FLAG);

  // Path
  const float alpha = isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.8f : 1.0f;
  path->setColor(12.0f / 255, 45.0f / 255,  83.0f / 255, alpha);
  path->update(gl);

  // Tool
  if (path.isSet() && path->getPath().isSet()) {
    const GCode::ToolTable &tools = path->getPath()->getTools();
    const GCode::Move &move = path->getMove();
    int toolID = move.getTool();

    if (tools.has(toolID)) {
      Vector3D position = path->getPosition();
      if (showMachine) position *= machine->getTool();

      tool->set(tools.get(toolID));
      tool->getTransform().toIdentity();
      tool->getTransform().translate(position);

    } else tool->clear();
  } else tool->clear();

  // Surface
  if (surfaceChanged) {
    surfaceChanged = false;

    if (surface.isSet()) {
      mesh->reset(surface->getTriangleCount());

      auto cb =
        [this] (const vector<float> &vertices, const vector<float> &normals) {
          mesh->add(vertices, normals);
        };

      surface->getVertices(cb);

    } else mesh->reset(0);
  }

#if 0 // TODO GL

  //Vector3D currentPosition = path->getPosition();

  if (showMachine) {
    //Vector3D v = machine->getWorkpiece() * currentPosition;
    //gl.glTranslatef(v.x(), v.y(), v.z());
  }

  // Line normals relative to camera rotation
  //const double *rotation = getRotation();
  //gl.glNormal3f(rotation[1], rotation[2], rotation[3]);

  // Model
  //const float ambient[] = {12.0f / 255, 45.0f / 255,  83.0f / 255, alpha};
  //const float diffuse[] = {16.0f / 255, 59.0f / 255, 108.0f / 255, alpha};

  //gl.glDisable(GL_COLOR_MATERIAL);
  //gl.glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
  //gl.glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

  //setWire(isFlagSet(View::WIRE_FLAG));

  // Bounding box tree
  if (isFlagSet(View::SHOW_BBTREE_FLAG) && !moveLookup.isNull())
    moveLookup->draw(isFlagSet(View::BBTREE_LEAVES_FLAG));

  // Machine
  if (showMachine) {
    // TODO Work offsets should be configurable
    //gl.glTranslatef(0, 0, -workpieceBounds.getDimensions().z());

    machine->setPosition(currentPosition);
    machine->draw(isFlagSet(View::WIRE_FLAG));
  }
#endif

  GLScene::glDraw(gl);
}
