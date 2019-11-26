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

#include "JSONMachine.h"

#include <cbang/Exception.h>

using namespace cb;
using namespace std;
using namespace GCode;


void JSONMachine::start() {
  filename.clear();
  line = -1;

  MachineAdapter::start();

  sink.beginList();

  sink.appendDict(true);
  sink.insert("type", "units");
  sink.insert("value", units.toString());
  sink.endDict();
}


void JSONMachine::end() {
  MachineAdapter::end();
  sink.endList();
}


void JSONMachine::setFeed(double feed) {
  double oldFeed = getFeed();
  MachineAdapter::setFeed(feed);

  if (feed != oldFeed) {
    sink.appendDict(true);
    sink.insert("type", "feed");
    sink.insert("value", feed);
    sink.endDict();
  }
}


void JSONMachine::setFeedMode(feed_mode_t mode) {
  feed_mode_t oldMode = getFeedMode();
  MachineAdapter::setFeedMode(mode);

  if (oldMode != mode) {
    sink.appendDict(true);
    sink.insert("type", "feed-mode");
    sink.insert("value", MachineEnum::toString(mode));
    sink.endDict();
  }
}


void JSONMachine::setSpeed(double speed) {
  double oldSpeed = getSpeed();
  MachineAdapter::setSpeed(speed);

  if (oldSpeed != speed) {
    sink.appendDict(true);
    sink.insert("type", "speed");
    sink.insert("value", speed);
    sink.endDict();
  }
}


void JSONMachine::setSpinMode(spin_mode_t mode, double max) {
  double oldMax = 0;
  spin_mode_t oldMode = getSpinMode(&oldMax);

  MachineAdapter::setSpinMode(mode, max);

  if (oldMode != mode || (oldMax != max && mode == CONSTANT_SURFACE_SPEED)) {
    sink.appendDict(true);
    sink.insert("type", "spin-mode");
    sink.insert("value", MachineEnum::toString(mode));
    if (mode == CONSTANT_SURFACE_SPEED && max) sink.insert("max", max);
    sink.endDict();
  }
}


void JSONMachine::setPathMode(path_mode_t mode, double motionBlending,
                              double naiveCAM) {
  MachineAdapter::setPathMode(mode, motionBlending, naiveCAM);

  sink.appendDict(true);
  sink.insert("type", "path-mode");
  sink.insert("value", MachineEnum::toString(mode));

  if (mode == CONTINUOUS_MODE) {
    if (0 < motionBlending) sink.insert("blend", motionBlending);
    if (0 < naiveCAM) sink.insert("naive-cam", naiveCAM);
  }

  sink.endDict();
}


void JSONMachine::changeTool(unsigned tool) {
  unsigned currentTool = get(TOOL_NUMBER, NO_UNITS);

  MachineAdapter::changeTool(tool);

  if (tool != currentTool) {
    sink.appendDict(true);
    sink.insert("type", "tool");
    sink.insert("value", tool);
    sink.endDict();
  }
}


void JSONMachine::input(port_t port, input_mode_t mode, double timeout) {
  MachineAdapter::input(port, mode, timeout);

  sink.appendDict(true);
  sink.insert("type", "input");
  sink.insert("port", MachineEnum::toString(port));
  sink.insert("mode", MachineEnum::toString(mode));
  sink.insert("timeout", timeout);
  sink.endDict();
}


void JSONMachine::seek(port_t port, bool active, bool error) {
  MachineAdapter::seek(port, active, error);

  sink.appendDict(true);
  sink.insert("type", "seek");
  sink.insert("port", MachineEnum::toString(port));
  sink.insertBoolean("active", active);
  sink.insertBoolean("error", error);
  sink.endDict();
}


void JSONMachine::output(port_t port, double value) {
  MachineAdapter::output(port, value);

  sink.appendDict(true);
  sink.insert("type", "output");
  sink.insert("port", MachineEnum::toString(port));
  sink.insert("value", value);
  sink.endDict();
}


void JSONMachine::dwell(double seconds) {
  MachineAdapter::dwell(seconds);

  sink.appendDict(true);
  sink.insert("type", "dwell");
  sink.insert("seconds", seconds);
  sink.endDict();
}


void JSONMachine::move(const Axes &_target, int axes, bool rapid) {
  MachineAdapter::move(_target, axes, rapid);

  Axes target = getTransforms().transform(_target);

  sink.appendDict(true);
  sink.insert("type", "move");
  if (rapid) sink.insertBoolean("rapid", true);

  sink.insertDict("to", true);
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (axes & MachineEnum::getVarType(*axis))
      sink.insert(string(1, tolower(*axis)), target.get(*axis));

  sink.endDict();
  sink.endDict();
}


void JSONMachine::arc(const Vector3D &offset, const Vector3D &target,
                      double angle, plane_t plane) {
  MachineAdapter::arc(offset, target, angle, plane);

  sink.appendDict(true);
  sink.insert("type", "arc");
  sink.beginInsert("offset");
  offset.write(sink);
  sink.beginInsert("to");
  target.write(sink);
  sink.insert("angle", angle);
  sink.insert("plane", MachineEnum::toString(plane));
  sink.endDict();
}


void JSONMachine::pause(pause_t type) {
  MachineAdapter::pause(type);

  sink.appendDict(true);
  sink.insert("type", "pause");
  if (type == PAUSE_OPTIONAL) sink.insertBoolean("optional", true);
  if (type == PAUSE_PALLET_CHANGE) sink.insertBoolean("pallet-change", true);
  sink.endDict();
}


void JSONMachine::setLocation(const cb::LocationRange &location) {
  MachineAdapter::setLocation(location);

  if (!withLocation) return;

  if (location.getStart().getFilename() != filename) {
    filename = location.getStart().getFilename();

    sink.appendDict(true);
    sink.insert("type", "source");
    sink.insert("value", filename);
    sink.endDict();
  }

  if (location.getStart().getLine() != line) {
    line = location.getStart().getLine();

    sink.appendDict(true);
    sink.insert("type", "line");
    sink.insert("value", line);
    sink.endDict();
  }
}


void JSONMachine::comment(const string &s) const {
  MachineAdapter::comment(s);

  sink.appendDict(true);
  sink.insert("type", "comment");
  sink.insert("value", s);
  sink.endDict();
}


void JSONMachine::message(const string &s) {
  MachineAdapter::message(s);

  sink.appendDict(true);
  sink.insert("type", "message");
  sink.insert("value", s);
  sink.endDict();
}
