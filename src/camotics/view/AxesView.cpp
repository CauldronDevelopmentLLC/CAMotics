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
#include "GLSphere.h"

using namespace CAMotics;
using namespace cb;


namespace {
  float toRadians(float degrees) {return degrees * Math::PI / 180;}
}


AxesView::AxesView() {
  add(new AxisView(Color(1, 0.00, 0.0), toRadians(90),  Vector3D(0, 1, 0)));
  add(new AxisView(Color(0, 0.75, 0.1), toRadians(270), Vector3D(1, 0, 0)));
  add(new AxisView(Color(0, 0.00, 1.0), toRadians(0),   Vector3D(1, 0, 0)));

  SmartPointer<GLSphere> center = new GLSphere(0.07, 64, 64);
  center->setColor(0.2, 0.2, 0.2);
  add(center);
}
