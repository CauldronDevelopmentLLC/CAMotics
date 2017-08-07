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

#include "ToolView.h"
#include "GL.h"

using namespace CAMotics;


namespace {
  void drawCylinder(double radiusA, double radiusB, double length) {
    // Body
    glCylinder(radiusA, radiusB, length, 128);

    // End caps
    if (radiusA) {
      glNormal3f(0, 0, -1);
      glDisk(radiusA, 128);
    }

    if (radiusB) {
      glPushMatrix();
      glTranslatef(0, 0, length);
      glNormal3f(0, 0, 1);
      glDisk(radiusB, 128);
      glPopMatrix();
    }
  }
}


void ToolView::draw() {
  double diameter = tool.getDiameter();
  double radius = tool.getRadius();
  double length = tool.getLength();
  GCode::ToolShape shape = tool.getShape();

  glPushMatrix();
  glTranslatef(position.x(), position.y(), position.z());

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
    glSphere(radius, 128, 128);
    glPopMatrix();
    break;
  }

  case GCode::ToolShape::TS_BALLNOSE:
    glPushMatrix();
    glTranslatef(0, 0, radius);
    glSphere(radius, 128, 128);
    drawCylinder(radius, radius, length - radius);
    glPopMatrix();
    break;

  case GCode::ToolShape::TS_SNUBNOSE:
    drawCylinder(tool.getSnubDiameter() / 2, radius, length);
    break;

  case GCode::ToolShape::TS_CONICAL: drawCylinder(0, radius, length); break;

  case GCode::ToolShape::TS_CYLINDRICAL:
  default: drawCylinder(radius, radius, length); break;
  }

  glPopMatrix();
}
