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

#include "AxisView.h"

#include "GLCylinder.h"
#include "GLDisk.h"

using namespace CAMotics;
using namespace cb;


AxisView::AxisView() {
  // Shaft
  add(new GLCylinder(0.05, 0.05, 1, 128));

  // Head
  SmartPointer<GLComposite> c = new GLComposite;
  c->getTransform().translate(0, 0, 1);
  c->add(new GLDisk(0.075, 128));
  c->add(new GLCylinder(0.075, 0, 0.1, 128));
  add(c);
}
