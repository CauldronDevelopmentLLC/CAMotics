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
#include <cbang/Catch.h>
#include <cbang/util/HumanNumber.h>
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
  bool wire = isFlagSet(View::WIRE_FLAG);

  axes->setVisible(isFlagSet(View::SHOW_AXES_FLAG));
  bounds->setVisible(!isFlagSet(View::SHOW_WORKPIECE_FLAG) &&
                     isFlagSet(View::SHOW_WORKPIECE_BOUNDS_FLAG));
  tool->setVisible(isFlagSet(View::SHOW_TOOL_FLAG) && !path->isEmpty());
  path->setShowIntensity(isFlagSet(View::PATH_INTENSITY_FLAG));
  path->setVisible(isFlagSet(View::SHOW_PATH_FLAG));
  model->setVisible(isFlagSet(View::SHOW_SURFACE_FLAG) && !wire);
  wireModel->setVisible(isFlagSet(View::SHOW_SURFACE_FLAG) && wire);
  workpiece->setVisible(isFlagSet(View::SHOW_WORKPIECE_FLAG));
  machineView->setVisible(isFlagSet(View::SHOW_MACHINE_FLAG));
  machineView->setWire(wire);
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


void View::updateTool() {
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
}


void View::loadWireModel() {
  wireModel->reset(surface->getTriangleCount() * 3, false, true);

  auto cb =
    [this] (const vector<float> &vertices, const vector<float> &normals) {
      unsigned triangles = vertices.size() / 9;
      vector<float> v(triangles * 18);
      vector<float> n(triangles * 18);

      for (unsigned i = 0; i < triangles; i++)
        for (unsigned j = 0; j < 2; j++) {
          const float *t = j ? &normals[i * 9] : &vertices[i * 9];
          vector<float> &out = j ? n : v;
          const float *v0 = t;
          const float *v1 = &t[3];
          const float *v2 = &t[6];

          memcpy(&out[i * 18 +  0], v0, sizeof(float) * 6);
          memcpy(&out[i * 18 +  6], v1, sizeof(float) * 6);
          memcpy(&out[i * 18 + 12], v2, sizeof(float) * 3);
          memcpy(&out[i * 18 + 15], v0, sizeof(float) * 3);
        }

      wireModel->add(triangles * 6, &v[0], 0, &n[0]);
    };

  surface->getVertices(cb);
  wireModel->setLight(true);
}


void View::updateModel() {
  // Color
  const float alpha = isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.5f : 1.0f;
  model->getColor().setAlpha(alpha);

  bool wire = isFlagSet(View::WIRE_FLAG);

  if (surfaceChanged) {
    surfaceChanged = false;
    wireModel->reset(0, false, false);

    if (surface.isSet()) {
      model->reset(surface->getTriangleCount());

      auto cb =
        [this] (const vector<float> &vertices, const vector<float> &normals) {
          model->add(vertices, normals);
        };

      surface->getVertices(cb);
      model->setVisible(!wire);

    } else model->reset(0);
  }

  if (surface.isSet() && wire && wireModel->empty()) loadWireModel();
}


void View::updateMachine() {
  group->getTransform().toIdentity();

  if (machineView->isVisible()) {
    if (machineChanged) {
      machineChanged = false;
      machineView->load(*machine);
      machineView->setWire(isFlagSet(View::WIRE_FLAG));
    }

    Vector3D currentPosition = path->getPosition();
    Vector3D offset = machine->getWorkpiece() * currentPosition;
    group->getTransform().translate(offset);

    // TODO machine/tool transform is incorrect
    // TODO Work offsets should be configurable
    double z = -workpieceBounds.getDimensions().z();
    auto &t = machineView->getTransform();
    t.toIdentity();
    t.translate(0, 0, z);

    machineView->setPosition(currentPosition);
  }
}


void View::updateAABB() {
  if (moveLookup.isSet() && aabbView->isVisible() && moveLookupChanged) {
    moveLookupChanged = false;
    aabbView->load(*moveLookup.cast<AABBTree>());
  }
}


void View::glInit() {
  GLScene::glInit();

  // Build scene
  add(machineView);
  add(group = new GLComposite);
  group->add(axes = new AxesView);
  group->add(bounds = new GLBox);
  group->add(path);
  group->add(aabbView);
  group->add(model = new Mesh(0));
  group->add(wireModel = new Lines(0, false, false));
  group->add(workpiece = new CuboidView);
  group->add(tool = new ToolView); // Last for transparency
  group->setPickable(true);

  // Colors
  Color modelColor(0.06, 0.23, 0.42);
  model->setColor(modelColor);
  wireModel->setColor(modelColor);
  workpiece->setColor(modelColor);
  bounds->setColor(1, 1, 1, 0.5); // White
}


void View::glDraw(bool picking) {
  updateVisibility();
  updateBounds();
  path->update();
  updateTool();
  updateModel();
  updateMachine();
  updateAABB();

  GLScene::glDraw(picking);
}
