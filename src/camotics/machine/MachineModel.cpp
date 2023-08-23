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


namespace {
  Vector3U getDefaultColor(const string &name) {
    if (name == "x") return Vector3U(25, 25, 200);
    if (name == "y") return Vector3U(200, 200, 25);
    if (name == "z") return Vector3U(200, 25, 25);
    if (name == "a") return Vector3U(200, 230, 159);
    if (name == "b") return Vector3U(200, 138, 82);
    if (name == "f") return Vector3U(144, 238, 144);
    if (name == "general") return Vector3U(200, 165, 25);
    if (name == "h") return Vector3U(135, 206, 235);
    if (name == "m") return Vector3U(207, 103, 235);
    if (name == "p") return Vector3U(144, 238, 144);
    if (name == "r") return Vector3U(224, 204, 235);

    return Vector3U(100, 100, 100);
  }
}


MachineModel::MachineModel(const InputSource &source) :
  path(source.getName()) {read(source.getStream());}


void MachineModel::add(const cb::SmartPointer<MachinePart> &part) {
  parts[part->getName()] = part;
  bounds.add(part->getBounds());
}


void MachineModel::readTCO(
  const InputSource &source, const JSON::Value &config) {
  auto &partConfigs = config.getDict("parts");

  Matrix4x4D transform;
  if (config.hasList("transform")) {
    auto &t = config.getList("transform");

    for (int i = 0; i < 4; i++)
      transform[i].read(t.getList(i));

  } else transform.toIdentity();

  bool reverseWinding = config.getBoolean("reverse_winding", false);

  while (source.getStream().good()) {
    string line = String::trim(source.getLine());

    if (!line.empty() && line[0] == '[') {
      string name = String::toLower(line.substr(1, line.length() - 2));
      SmartPointer<JSON::Value> partConfig;

      if (partConfigs.hasDict(name)) partConfig = partConfigs.get(name);
      else partConfig = new JSON::Dict;

      if (!partConfig->has("color"))
        partConfig->insert("color", getDefaultColor(name).getJSON());

      SmartPointer<MachinePart> part = new MachinePart(name, partConfig);
      part->readTCO(source, transform, reverseWinding);
      if (part->getTriangleCount()) add(part);
    }
  }
}


void MachineModel::read(const JSON::Value &value) {
  name = value.getString("name");
  tool.read(value.getList("tool"));
  workpiece.read(value.getList("workpiece"));

  if (value.hasString("model")) {
    string model = SystemUtilities::absolute(path, value.getString("model"));
    readTCO(model, value);

  } else if (value.hasDict("parts")) {
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
