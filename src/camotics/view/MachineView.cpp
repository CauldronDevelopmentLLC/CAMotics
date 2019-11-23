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

#include "MachineView.h"

#include <camotics/machine/MachineModel.h>

using namespace CAMotics;
using namespace cb;
using namespace std;


void MachineView::load(MachineModel &model) {
  clear();
  parts.clear();

  for (auto it = model.begin(); it != model.end(); it++) {
    SmartPointer<MachinePartView> part = new MachinePartView(*it->second);
    add(part);
    parts.push_back(part);
  }
}


void MachineView::setWire(bool wire) {
  for (unsigned i = 0; i < parts.size(); i++)
    parts[i]->setWire(wire);
}


void MachineView::setPosition(const Vector3D &position) {
  for (unsigned i = 0; i < parts.size(); i++)
    parts[i]->setPosition(position);
}
