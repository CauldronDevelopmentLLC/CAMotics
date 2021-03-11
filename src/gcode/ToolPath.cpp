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

#include "ToolPath.h"

#include <cbang/json/Sink.h>
#include <cbang/json/Dict.h>

#include <string>
#include <limits>

using namespace std;
using namespace cb;
using namespace GCode;


ToolPath::~ToolPath() {}


int ToolPath::find(double time, unsigned first, unsigned last) const {
  // Base case, empty list
  if (first == last) return -1;

  // Base case, one item
  if (first == last - 1) {
    if (at(first).getStartTime() <= time && time <= at(first).getEndTime())
      return first;

    return -1;
  }

  // Recur
  unsigned mid = (first + last) / 2;
  if (time < at(mid).getStartTime()) return find(time, first, mid);
  return find(time, mid, last);
}


int ToolPath::find(double time) const {return find(time, 0, size());}


void ToolPath::read(const JSON::Value &value) {
  GCode::Axes start;
  GCode::MoveType type = GCode::MoveType::MOVE_RAPID;
  int line = 0;
  int tool = 1;
  double feed = 0;
  double speed = 0;
  double time = 0;

  for (unsigned i = 0; i < value.size(); i++) {
    auto &dict = value.getDict(i);

    GCode::Axes end;
    for (int j = 0; j < 9; j++)
      end[j] = dict.getNumber(string(1, GCode::Axes::toAxis(j)), start[j]);

    if (dict.hasString("type"))
      type = GCode::MoveType::parse(dict.getString("type"));
    line = dict.getNumber("line", line);
    tool = dict.getNumber("tool", tool);
    feed = dict.getNumber("feed", feed);
    speed = dict.getNumber("speed", speed);
    double delta = dict.getNumber("time", 0);

    GCode::Move m(type, start, end, time, tool, feed, speed, line, delta);
    move(m);

    time += m.getTime();
    start = end;
  }
}


void ToolPath::write(JSON::Sink &sink) const {
  GCode::Axes lastPos(numeric_limits<double>::infinity());
  GCode::MoveType type = (GCode::MoveType::enum_t)-1;
  int line = -1;
  int tool = -1;
  double feed = -1;
  double speed = -1;

  sink.beginList();
  for (unsigned i = 0; i < size(); i++) {
    const GCode::Move &move = at(i);
    sink.appendDict(true);

    // Axes
    for (unsigned j = 0; j < 9; j++)
      if (move.getEnd()[j] != lastPos[j])
        sink.insert(string(1, GCode::Axes::toAxis(j)),
                    lastPos[j] = move.getEnd()[j]);

    // Type
    if (type != move.getType())
      sink.insert("type", (type = move.getType()).toString());

    // Line number
    if (line != (int)move.getLine())
      sink.insert("line", line = move.getLine());

    // GCode::Tool
    if (tool != (int)move.getTool())
      sink.insert("tool", tool = move.getTool());

    // Feed
    if (feed != move.getFeed())
      sink.insert("feed", feed = move.getFeed());

    // Speed
    if (speed != move.getSpeed())
      sink.insert("speed", speed = move.getSpeed());

    sink.insert("time", move.getTime());

    sink.endDict();
  }

  sink.endList();
}


void ToolPath::move(GCode::Move &move) {
  push_back(move);

  // Bounds
  Rectangle3D::add(move.getStartPt());
  Rectangle3D::add(move.getEndPt());

  time += move.getTime();
  distance += move.getDistance();
}
