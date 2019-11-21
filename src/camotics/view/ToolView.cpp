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

#include "ToolView.h"

#include "GLSphere.h"
#include "GLConic.h"

using namespace CAMotics;
using namespace cb;


void ToolView::set(const GCode::Tool &tool) {
  clear();

  double diameter = tool.getDiameter();
  double radius = tool.getRadius();
  double length = tool.getLength();
  GCode::ToolShape shape = tool.getShape();

  // TODO semitransparent tool

  if (radius <= 0) {
    // Default tool specs
    radius = 25.4 / 8;
    shape = GCode::ToolShape::TS_CONICAL;
    setColor(1, 0, 0); // Red

  } else setColor(1, 0.5, 0); // Orange

  if (length <= 0) length = 50;

  switch (shape) {
  case GCode::ToolShape::TS_SPHEROID: {
    getTransform().translate(0, 0, length / 2);
    getTransform().scale(1, 1, length / diameter);
    add(new GLSphere(radius, 128, 128));
    break;
  }

  case GCode::ToolShape::TS_BALLNOSE:
    getTransform().translate(0, 0, radius);
    add(new GLSphere(radius, 128, 128));
    add(new GLConic(radius, radius, length - radius));
    break;

  case GCode::ToolShape::TS_SNUBNOSE:
    add(new GLConic(tool.getSnubDiameter() / 2, radius, length));
    break;

  case GCode::ToolShape::TS_CONICAL:
    add(new GLConic(0, radius, length));
    break;

  case GCode::ToolShape::TS_CYLINDRICAL:
  default: add(new GLConic(radius, radius, length)); break;
  }
}
