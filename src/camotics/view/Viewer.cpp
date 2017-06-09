/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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


static void drawCylinder(GLUquadric *quad, double radiusA, double radiusB,
                         double length) {
  // Body
  gluCylinder(quad, radiusA, radiusB, length, 100, 100);

  // End caps
  if (radiusA) gluDisk(quad, 0, radiusA, 100, 100);
  if (radiusB) {
    glPushMatrix();
    glTranslatef(0, 0, length);
    gluDisk(quad, 0, radiusB, 100, 100);
    glPopMatrix();
  }
}


void Viewer::init() {
  if (!toolQuad) {
    toolQuad = gluNewQuadric();
    if (!toolQuad) THROW("Failed to allocate tool quad");
  }
}


void Viewer::draw(const View &view) {
  init();

  SmartPointer<Surface> surface = view.surface;

  // Setup view port
  cb::Rectangle3D bounds = view.path->getBounds();
  if (!surface.isNull()) bounds.add(surface->getBounds());
  bounds.add(view.workpiece->getBounds());
  view.glDraw(bounds);

  // Enable Lighting
  view.setLighting(true);

  // Workpiece bounds
  if (!view.isFlagSet(View::SHOW_WORKPIECE_FLAG) &&
      view.isFlagSet(View::SHOW_WORKPIECE_BOUNDS_FLAG)) {
    glLineWidth(1);
    glColor4f(1, 1, 1, 0.5); // White

    BoundsView(view.workpiece->getBounds()).draw();
  }

  // GCode::Tool path
  if (view.isFlagSet(View::SHOW_PATH_FLAG)) view.path->draw();
  else view.path->update();

  // Model
  if (view.isFlagSet(View::SHOW_WORKPIECE_FLAG | View::SHOW_SURFACE_FLAG)) {
    const float alpha =
      view.isFlagSet(View::TRANSLUCENT_SURFACE_FLAG) ? 0.8 : 1;
    const float ambient[] = {12.0 / 255, 45.0 / 255,  83.0 / 255, alpha};
    const float diffuse[] = {16.0 / 255, 59.0 / 255, 108.0 / 255, alpha};

    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

    view.setWire(view.isFlagSet(View::WIRE_FLAG));

    if (!surface.isNull() && view.isFlagSet(View::SHOW_SURFACE_FLAG))
      surface->draw(view.isFlagSet(View::SURFACE_VBOS_FLAG));

    if (view.isFlagSet(View::SHOW_WORKPIECE_FLAG)) view.workpiece->draw();

    glEnable(GL_COLOR_MATERIAL);

    view.setWire(false);
  }

  // Bounding box tree
  if (view.isFlagSet(View::SHOW_BBTREE_FLAG) && !view.moveLookup.isNull())
    view.moveLookup->draw(view.isFlagSet(View::BBTREE_LEAVES_FLAG));

  // GCode::Tool
  if (view.isFlagSet(View::SHOW_TOOL_FLAG) && !view.path->isEmpty()) {
    const GCode::ToolTable &tools = view.path->getPath()->getTools();
    const GCode::Move &move = view.path->getMove();
    int toolID = move.getTool();

    if (tools.has(toolID)) {
      const GCode::Tool &tool = tools.get(toolID);
      double diameter = tool.getDiameter();
      double radius = tool.getRadius();
      double length = tool.getLength();
      GCode::ToolShape shape = tool.getShape();

      cb::Vector3D currentPosition = view.path->getPosition();
      glTranslatef(currentPosition.x(), currentPosition.y(),
                   currentPosition.z());

      if (radius <= 0) {
        // Default tool specs
        radius = 25.4 / 8;
        shape = GCode::ToolShape::TS_CONICAL;

        glColor4f(1, 0, 0, 1); // Red

      } else glColor4f(1, 0.5, 0, 1); // Orange

      if (length <= 0) length = 50;

      switch (shape) {
      case GCode::ToolShape::TS_SPHEROID: {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(0, 0, length / 2);
        glScaled(1, 1, length / diameter);
        gluSphere((GLUquadric *)toolQuad, radius, 100, 100);
        glPopMatrix();
        break;
      }

      case GCode::ToolShape::TS_BALLNOSE:
        glPushMatrix();
        glTranslatef(0, 0, radius);
        gluSphere((GLUquadric *)toolQuad, radius, 100, 100);
        drawCylinder((GLUquadric *)toolQuad, radius, radius, length - radius);
        glPopMatrix();
        break;

      case GCode::ToolShape::TS_SNUBNOSE:
        drawCylinder((GLUquadric *)toolQuad, tool.getSnubDiameter() / 2, radius,
                     length);
        break;

      case GCode::ToolShape::TS_CONICAL:
        drawCylinder((GLUquadric *)toolQuad, 0, radius, length);
        break;

      case GCode::ToolShape::TS_CYLINDRICAL:
      default:
        drawCylinder((GLUquadric *)toolQuad, radius, radius, length);
        break;
      }
    }
  }

  // Disable Lighting
  view.setLighting(false);

  CHECK_GL_ERROR("");
}
