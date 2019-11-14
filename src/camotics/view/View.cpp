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
#include "GL.h"
#include "BoundsView.h"
#include "ToolView.h"

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
  speed(1), reverse(false), lastTime(0), path(new ToolPathView(valueSet)),
  workpiece(new CuboidView) {

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
  path->update(isFlagSet(View::PATH_INTENSITY_FLAG));
}


void View::setWorkpiece(const Rectangle3D &bounds) {
  workpiece->setBounds(bounds);
}


void View::setSurface(const SmartPointer<Surface> &surface) {
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
  setSurface(0);
  setMoveLookup(0);
  path->setByRatio(1);
  resetView();
}


void View::draw() {
  GLFuncs &glFuncs = getGLFuncs();

  bool showMachine = !machine.isNull() && isFlagSet(View::SHOW_MACHINE_FLAG);

  // Setup view port
  Rectangle3D bounds = path->getBounds();
  if (!surface.isNull()) bounds.add(surface->getBounds());
  bounds.add(workpiece->getBounds());
  if (showMachine) bounds.add(machine->getBounds());
  glDraw(bounds, workpiece->getBounds().getCenter());

  // Enable Lighting
  setLighting(true);

  Vector3D currentPosition = path->getPosition();
  // glFuncs.glPushMatrix();

  if (showMachine) {
    //Vector3D v = machine->getWorkpiece() * currentPosition;
    //glFuncs.glTranslatef(v.x(), v.y(), v.z());
  }

  // Axes
  if (isFlagSet(View::SHOW_AXES_FLAG))
    drawAxes(workpiece->getBounds());

  // Line normals relative to camera rotation
  //const double *rotation = getRotation();
  //glFuncs.glNormal3f(rotation[1], rotation[2], rotation[3]);

  // Workpiece bounds
  if (!isFlagSet(View::SHOW_WORKPIECE_FLAG) &&
      isFlagSet(View::SHOW_WORKPIECE_BOUNDS_FLAG)) {
    glFuncs.glLineWidth(1);
    //glFuncs.glColor4f(1, 1, 1, 0.5); // White

    BoundsView(workpiece->getBounds()).draw();
  }

  // GCode::Tool path
  path->update(isFlagSet(View::PATH_INTENSITY_FLAG));
  if (isFlagSet(View::SHOW_PATH_FLAG)) path->draw();

  // Model
  if (isFlagSet(View::SHOW_WORKPIECE_FLAG | View::SHOW_SURFACE_FLAG)) {
    //const float alpha =
    //  isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.8f : 1.0f;
    //const float ambient[] = {12.0f / 255, 45.0f / 255,  83.0f / 255, alpha};
    //const float diffuse[] = {16.0f / 255, 59.0f / 255, 108.0f / 255, alpha};

    //glFuncs.glDisable(GL_COLOR_MATERIAL);
    //glFuncs.glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    //glFuncs.glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

    setWire(isFlagSet(View::WIRE_FLAG));

    if (!surface.isNull() && isFlagSet(View::SHOW_SURFACE_FLAG))
      surface->draw();

    if (isFlagSet(View::SHOW_WORKPIECE_FLAG)) workpiece->draw();

    //glFuncs.glEnable(GL_COLOR_MATERIAL);

    setWire(false);
  }

  // Bounding box tree
  if (isFlagSet(View::SHOW_BBTREE_FLAG) && !moveLookup.isNull())
    moveLookup->draw(isFlagSet(View::BBTREE_LEAVES_FLAG));

  //glFuncs.glPopMatrix();

  // Machine
  if (showMachine) {
    //glFuncs.glPushMatrix();

    // TODO Work offsets should be configurable
    //glFuncs.glTranslatef(0, 0, -workpiece->getBounds().getDimensions().z());

    machine->setPosition(currentPosition);
    machine->draw(isFlagSet(View::WIRE_FLAG));

    //glFuncs.glPopMatrix();
  }

  // GCode::Tool
  if (isFlagSet(View::SHOW_TOOL_FLAG) && !path->isEmpty()) {
    const GCode::ToolTable &tools = path->getPath()->getTools();
    const GCode::Move &move = path->getMove();
    int toolID = move.getTool();

    if (tools.has(toolID)) {
      Vector3D position = currentPosition;
      if (showMachine) position *= machine->getTool();
      ToolView(tools.get(toolID), position).draw();
    }
  }

  // Disable Lighting
  setLighting(false);
}
