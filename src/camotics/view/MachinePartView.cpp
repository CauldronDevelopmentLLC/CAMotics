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

#include "MachinePartView.h"

#include <camotics/machine/MachinePart.h>

using namespace CAMotics;
using namespace std;
using namespace cb;


MachinePartView::MachinePartView(MachinePart &part) :
  movement(part.getMovement()), offset(part.getOffset()) {

  // Color
  const Vector3U &c = part.getColor();
  color = Color(c[0] / 255.0, c[1] / 255.0, c[2] / 255.0);

  // Lines
  add(lines = new Lines(part.getLines()));

  // Mesh
  add(mesh = new Mesh(part.getTriangleCount()));
  mesh->setColor(color);

  auto cb =
    [this] (const vector<float> &vertices, const vector<float> &normals) {
      mesh->add(vertices, normals);
    };

  part.getVertices(cb);
}


void MachinePartView::setWire(bool wire) {
  if (wire) lines->setColor(color);
  else lines->setColor(color * 0.8);
  mesh->setVisible(!wire);
}


void MachinePartView::setPosition(const Vector3D &position) {
  auto &t = getTransform();
  t.toIdentity();
  t.translate(offset);
  t.translate(position * movement);
}
