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

#include "AxesView.h"
#include "AxisView.h"

using namespace CAMotics;
using namespace cb;


namespace {
  float toRadians(float degrees) {return degrees * M_PI / 180;}
}


AxesView::AxesView() {
  for (int axis = 0; axis < 3; axis++)
    for (int up = 0; up < 2; up++)
      addAxis(axis, up);
}


void AxesView::addAxis(int axis, bool up) {
  SmartPointer<GLObject> o = new AxisView;

  switch (axis) {
  case 0:
    o->setColor(1, 0, 0); // Red
    o->getTransform().rotate(toRadians(up ? 90 : 270), 0, 1, 0);
    break;

  case 1:
    o->setColor(0, 0.75, 0.1); // Green
    o->getTransform().rotate(toRadians(up ? 270 : 90), 1, 0, 0);
    break;

  case 2:
    o->setColor(0, 0, 1); // Blue
    o->getTransform().rotate(toRadians(up ? 0 : 180), 1, 0, 0);
    break;
  }

  add(o);
}
