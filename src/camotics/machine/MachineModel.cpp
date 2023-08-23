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

#include "MachineModel.h"

#include <cbang/json/JSON.h>
#include <cbang/os/SystemUtilities.h>

using namespace CAMotics;
using namespace cb;
using namespace std;


void MachineModel::add(const cb::SmartPointer<MachinePart> &part) {
  parts[part->getName()] = part;
  bounds.add(part->getBounds());
}


void MachineModel::read(const JSON::Value &value) {
  name = value.getString("name");
  tool.read(value.getList("tool"));
  workpiece.read(value.getList("workpiece"));

  if (value.hasDict("parts")) {
    auto &parts = *value.get("parts");

    for (unsigned i = 0; i < parts.size(); i++)
      add(new MachinePart(parts.keyAt(i), parts.get(i)));
  }
}


void MachineModel::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("name", name);

  sink.beginInsert("tool");
  tool.write(sink);

  sink.beginInsert("workpiece");
  workpiece.write(sink);

  sink.insertDict("parts");
  for (auto &it: parts) {
    sink.beginInsert(it.first);
    it.second->write(sink);
  }
  sink.endDict();

  sink.endDict();
}
