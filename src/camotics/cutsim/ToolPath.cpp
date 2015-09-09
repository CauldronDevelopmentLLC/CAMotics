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
using namespace tplang;
using namespace CAMotics;


ToolPath::~ToolPath() {}


void ToolPath::add(const Move &move) {
  push_back(move);

  // Bounds
  Rectangle3R::add(move.getStartPt());
  Rectangle3R::add(move.getEndPt());
}


void ToolPath::print(ostream &stream, bool metric) const {
  real lastFeed = 0;
  real lastSpeed = 0;
  real scale = metric ? 1 : (1 / 25.4);

  stream << (metric ? "G21 (metric)\n" : "G20 (imperial)\n");

  for (unsigned i = 0; i < size(); i++) {
    const Move &move = at(i);
    const Axes &start = move.getStart();
    const Axes &end = move.getEnd();

    // Move type
    switch (move.getType()) {
    case MoveType::MOVE_RAPID: stream << "G0"; break;
    case MoveType::MOVE_CUTTING: stream << "G1"; break;
    case MoveType::MOVE_PROBE: stream << "G1"; break; // TODO
    case MoveType::MOVE_DRILL: stream << "G1"; break; // TODO
    }

    // Axes
    for (unsigned axis = 0; axis < 9; axis++)
      if (start.getIndex(axis) != end.getIndex(axis)) {
        double value = end.getIndex(axis);
        if (fabs(value) < 0.0000000000001) value = 0;
        stream << ' ' << Axes::toAxis(axis)
               << String::printf("%.8g", value * scale);
      }

    // Feed
    if (move.getType() != MoveType::MOVE_RAPID) {
      if (move.getFeed() != lastFeed) stream << " F" << move.getFeed() * scale;
      lastFeed = move.getFeed();
    }

    // Speed
    if (move.getSpeed() != lastSpeed) stream << " S" << move.getSpeed();
    lastSpeed = move.getSpeed();

    stream << '\n';
  }
}


void ToolPath::write(JSON::Sink &sink) const {
  tplang::Axes lastPos(numeric_limits<double>::infinity());
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
        sink.insert(string(1, tplang::Axes::toAxis(j)),
                    lastPos[j] = move.getEnd()[j]);

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
