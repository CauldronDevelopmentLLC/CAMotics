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

#include "Viewer.h"

#include "View.h"
#include "GL.h"
#include "BoundsView.h"
#include "ToolView.h"

#include <gcode/ToolTable.h>
#include <camotics/sim/MoveLookup.h>

#include <cbang/String.h>
#include <cbang/Math.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/util/HumanNumber.h>
#include <cbang/time/HumanTime.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace cb;
using namespace CAMotics;


void Viewer::draw(const View &view) {
  GLFuncs &glFuncs = getGLFuncs();

  SmartPointer<Surface> surface = view.surface;
  bool showMachine =
    !view.machine.isNull() && view.isFlagSet(View::SHOW_MACHINE_FLAG);

  // Setup view port
  Rectangle3D bounds = view.path->getBounds();
  if (!surface.isNull()) bounds.add(surface->getBounds());
  bounds.add(view.workpiece->getBounds());
  if (showMachine) bounds.add(view.machine->getBounds());
  view.glDraw(bounds, view.workpiece->getBounds().getCenter());

  // Enable Lighting
  view.setLighting(true);

  Vector3D currentPosition = view.path->getPosition();
  glFuncs.glPushMatrix();

  if (showMachine) {
    Vector3D v = view.machine->getWorkpiece() * currentPosition;
    glFuncs.glTranslatef(v.x(), v.y(), v.z());
  }

  // Axes
  if (view.isFlagSet(View::SHOW_AXES_FLAG))
    view.drawAxes(view.workpiece->getBounds());

  // Line normals relative to camera rotation
  const double *rotation = view.getRotation();
  glFuncs.glNormal3f(rotation[1], rotation[2], rotation[3]);

  // Workpiece bounds
  if (!view.isFlagSet(View::SHOW_WORKPIECE_FLAG) &&
      view.isFlagSet(View::SHOW_WORKPIECE_BOUNDS_FLAG)) {
    glFuncs.glLineWidth(1);
    glFuncs.glColor4f(1, 1, 1, 0.5); // White

    BoundsView(view.workpiece->getBounds()).draw();
  }

  // GCode::Tool path
  view.path->update(view.isFlagSet(View::PATH_INTENSITY_FLAG));
  if (view.isFlagSet(View::SHOW_PATH_FLAG)) view.path->draw();

  // Model
  if (view.isFlagSet(View::SHOW_WORKPIECE_FLAG | View::SHOW_SURFACE_FLAG)) {
    const float alpha =
      view.isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.8 : 1;
    const float ambient[] = {12.0 / 255, 45.0 / 255,  83.0 / 255, alpha};
    const float diffuse[] = {16.0 / 255, 59.0 / 255, 108.0 / 255, alpha};

    glFuncs.glDisable(GL_COLOR_MATERIAL);
    glFuncs.glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glFuncs.glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

    view.setWire(view.isFlagSet(View::WIRE_FLAG));

    if (!surface.isNull() && view.isFlagSet(View::SHOW_SURFACE_FLAG))
      surface->draw(view.isFlagSet(View::SURFACE_VBOS_FLAG));

    if (view.isFlagSet(View::SHOW_WORKPIECE_FLAG)) view.workpiece->draw();

    glFuncs.glEnable(GL_COLOR_MATERIAL);

    view.setWire(false);
  }

  // Bounding box tree
  if (view.isFlagSet(View::SHOW_BBTREE_FLAG) && !view.moveLookup.isNull())
    view.moveLookup->draw(view.isFlagSet(View::BBTREE_LEAVES_FLAG));

  glFuncs.glPopMatrix();

  // Machine
  if (showMachine) {
    glFuncs.glPushMatrix();

    // TODO Work offsets should be configurable
    glFuncs.glTranslatef(0, 0,
                         -view.workpiece->getBounds().getDimensions().z());

    view.machine->setPosition(currentPosition);
    view.machine->draw(view.isFlagSet(View::SURFACE_VBOS_FLAG),
                       view.isFlagSet(View::WIRE_FLAG));

    glFuncs.glPopMatrix();
  }

  // GCode::Tool
  if (view.isFlagSet(View::SHOW_TOOL_FLAG) && !view.path->isEmpty()) {
    const GCode::ToolTable &tools = view.path->getPath()->getTools();
    const GCode::Move &move = view.path->getMove();
    int toolID = move.getTool();

    if (tools.has(toolID)) {
      Vector3D position = currentPosition;
      if (showMachine) position *= view.machine->getTool();
      ToolView(tools.get(toolID), position).draw();
    }
  }

  // Disable Lighting
  view.setLighting(false);

  CHECK_GL_ERROR("");
}
