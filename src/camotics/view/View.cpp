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
  path(new ToolPathView(valueSet)), aabbView(new AABBView),
  machineView(new MachineView) {

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
  path->update(); // Needed to compute simulation time
}


void View::setWorkpiece(const Rectangle3D &bounds) {
  workpieceBounds = bounds;
}


void View::setSurface(const SmartPointer<Surface> &surface) {
  surfaceChanged = true;
  this->surface = surface;
}


void View::setMoveLookup(const SmartPointer<MoveLookup> &moveLookup) {
  moveLookupChanged = true;
  this->moveLookup = moveLookup;
}


void View::setMachine(const SmartPointer<MachineModel> &machine) {
  machineChanged = true;
  this->machine = machine;
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
  model->setVisible(isFlagSet(View::SHOW_SURFACE_FLAG));
  workpiece->setVisible(isFlagSet(View::SHOW_WORKPIECE_FLAG));
  machineView->setVisible(isFlagSet(View::SHOW_MACHINE_FLAG));
  machineView->setWire(isFlagSet(View::WIRE_FLAG));
  aabbView->setVisible(isFlagSet(View::SHOW_BBTREE_FLAG));
  aabbView->showNodes(!isFlagSet(View::BBTREE_LEAVES_FLAG));
}


void View::updateBounds() {
  // Compute bounds
  Rectangle3D bbox = path->getBounds();
  if (surface.isSet()) bbox.add(surface->getBounds());
  bbox.add(workpieceBounds);
  if (machine.isSet() && machineView->isVisible())
    bbox.add(machine->getBounds());

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


void View::glInit() {
  GLScene::glInit();

  add(group = new GLComposite);
  add(machineView);

  group->add(axes = new AxesView);
  group->add(bounds = new GLBox);
  bounds->setColor(1, 1, 1, 0.5); // White
  group->add(tool = new ToolView);
  group->add(path);
  group->add(aabbView);

  // Model
  SmartPointer<Material> material =
    new Material(Color(0.05, 0.18, 0.33, 1), Color(0.06, 0.23, 0.42, 1));
  group->add(model = new Mesh(0));
  group->add(workpiece = new CuboidView);
  model->setMaterial(material);
  workpiece->setMaterial(material);
}


void View::glDraw() {
  updateVisibility();
  updateBounds();

  // Path
  path->update();

  // Model
  const float alpha = isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.8f : 1.0f;
  model->getMaterial()->ambient.setAlpha(alpha);
  model->getMaterial()->diffuse.setAlpha(alpha);

  // Tool
  if (path.isSet() && path->getPath().isSet()) {
    const GCode::ToolTable &tools = path->getPath()->getTools();
    const GCode::Move &move = path->getMove();
    int toolID = move.getTool();

    if (tools.has(toolID)) {
      Vector3D position = path->getPosition();
      if (machine.isSet() && machineView->isVisible())
        position *= machine->getTool();

      tool->set(tools.get(toolID));
      tool->getTransform().toIdentity();
      tool->getTransform().translate(position);

    } else tool->clear();
  } else tool->clear();

  // Surface
  if (surfaceChanged) {
    surfaceChanged = false;

    if (surface.isSet()) {
      model->reset(surface->getTriangleCount());

      auto cb =
        [this] (const vector<float> &vertices, const vector<float> &normals) {
          model->add(vertices, normals);
        };

      surface->getVertices(cb);

    } else model->reset(0);
  }

  // Machine
  group->getTransform().toIdentity();

  if (machineView->isVisible()) {
    if (machineChanged) {
      machineChanged = false;
      machineView->load(*machine);
    }

    Vector3D currentPosition = path->getPosition();
    Vector3D offset = machine->getWorkpiece() * currentPosition;
    group->getTransform().translate(offset);

    // TODO Work offsets should be configurable
    double z = -workpieceBounds.getDimensions().z();
    auto &t = machineView->getTransform();
    t.toIdentity();
    t.translate(0, 0, z);

    machineView->setPosition(currentPosition);
  }

  // AABB
  if (moveLookup.isSet() && aabbView->isVisible() && moveLookupChanged) {
    moveLookupChanged = false;
    aabbView->load(*moveLookup.cast<AABBTree>());
  }

  //setWire(isFlagSet(View::WIRE_FLAG));

  GLScene::glDraw();
}
