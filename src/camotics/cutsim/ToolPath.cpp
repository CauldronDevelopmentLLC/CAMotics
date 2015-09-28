/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "ToolPath.h"

#include <cbang/json/Sink.h>

#include <string>
#include <limits>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPath::~ToolPath() {}


void ToolPath::write(JSON::Sink &sink) const {
  Axes lastPos(numeric_limits<double>::infinity());
  MoveType type = (MoveType::enum_t)-1;
  int line = -1;
  int tool = -1;
  double feed = -1;
  double speed = -1;

  sink.beginList();
  for (unsigned i = 0; i < size(); i++) {
    const Move &move = at(i);
    sink.appendDict(true);

    // Axes
    for (unsigned j = 0; j < 9; j++)
      if (move.getEnd()[j] != lastPos[j])
        sink.insert(string(1, Axes::toAxis(j)), lastPos[j] = move.getEnd()[j]);

    // Type
    if (type != move.getType())
      sink.insert("type", (type = move.getType()).toString());

    // Line number
    if (line != (int)move.getLine())
      sink.insert("line", line = move.getLine());

    // Tool
    if (tool != (int)move.getTool())
      sink.insert("tool", tool = move.getTool());

    // Feed
    if (feed != move.getFeed())
      sink.insert("feed", feed = move.getFeed());

    // Speed
    if (speed != move.getSpeed())
      sink.insert("speed", speed = move.getSpeed());

    sink.endDict();
  }

  sink.endList();
}


void ToolPath::move(Move &move) {
  push_back(move);

  // Bounds
  Rectangle3R::add(move.getStartPt());
  Rectangle3R::add(move.getEndPt());
}
