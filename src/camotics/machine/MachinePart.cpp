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

#include "MachinePart.h"

using namespace CAMotics;
using namespace cb;
using namespace std;


MachinePart::MachinePart(const string &name, const JSON::ValuePtr &config) :
  name(name) {read(*config);}


void MachinePart::read(const JSON::Value &value) {
  color   .read(value.getList("color"));
  init    .read(value.getList("init"));
  home    .read(value.getList("home"));
  min     .read(value.getList("min"));
  max     .read(value.getList("max"));
  movement.read(value.getList("movement"));

  if (value.hasList("lines")) {
    auto &lines = value.getList("lines");
    for (auto &line: lines)
      this->lines.push_back(line->getNumber());
  }

  auto &vertices = value.getList("mesh");

  for (unsigned i = 0; i < vertices.size(); i += 9) {
    Vector3F t[3];

    for (unsigned j = 0; j < 3; j++)
      for (unsigned k = 0; k < 3; k++)
        t[j][k] = vertices.getNumber(i + j * 3 + k);

    add(t);
  }
}


void MachinePart::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.beginInsert("color");
  color.write(sink);

  sink.beginInsert("init");
  init.write(sink);

  sink.beginInsert("home");
  home.write(sink);

  sink.beginInsert("min");
  min.write(sink);

  sink.beginInsert("max");
  max.write(sink);

  sink.beginInsert("movement");
  movement.write(sink);

  if (!lines.empty()) {
    sink.insertList("lines");
    for (float x: lines) sink.append(x);
    sink.endList();
  }

  sink.insertList("mesh");
  for (auto &v: getVertices())
    sink.append(v);
  sink.endList();

  sink.endDict();
}
