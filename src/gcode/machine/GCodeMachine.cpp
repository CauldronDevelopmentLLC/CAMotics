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

#include "GCodeMachine.h"

#include <gcode/Plane.h>

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>

#include <limits>

using namespace cb;
using namespace std;
using namespace GCode;


namespace {
  struct dtos {
    double x;
    bool imperial;


    dtos(double x, bool imperial) : x(x), imperial(imperial) {
      if (Math::isnan(x))
        THROW("Numerical error in GCode stream:  NaN, caused by a divide by "
              "zero or other math error.");

      if (Math::isinf(x))
        THROW("Numerical error in GCode stream: Infinite value");
    }


    string toString() const {
      // TODO Get precision from config
      string s = String::printf("%.*f", imperial ? 4 : 3, x);

      int chop = 0;
      for (auto it = s.rbegin(); it != s.rend() && *it == '0'; it++)
        chop++;

      if (chop) s = s.substr(0, s.length() - chop);

      return s == "-0." ? "0." : s;
    }
  };


  inline ostream &operator<<(ostream &stream, const dtos &d) {
    return stream << d.toString();
  }
}


void GCodeMachine::beginLine() {
  const FileLocation &newLoc = getLocation().getStart();
  const string &filename = newLoc.getFilename();

  if (filename != location.getFilename()) {
    *stream << "(File: " << String::replace(filename, "\\)", "%29") << ")\n";
    location.setFilename(filename);
  }

  if (0 <= newLoc.getLine() && newLoc.getLine() != location.getLine()) {
    *stream << 'N' << newLoc.getLine() << ' ';
    location.setLine(newLoc.getLine());
  }
}


void GCodeMachine::start() {
  MachineAdapter::start();
  axisSeen = 0;
  *stream << (units == Units::METRIC ? "G21" : "G20") << '\n';
  // TODO set other GCode state
}


void GCodeMachine::end() {
  MachineAdapter::end();
  // Probably should be part of the machine description.
  *stream << "M2\n";
}


void GCodeMachine::setFeed(double feed) {
  double oldFeed = getFeed();
  MachineAdapter::setFeed(feed);

  if (feed != oldFeed) {
    beginLine();
    *stream << "F" << dtos(feed, false) << '\n';
  }
}


void GCodeMachine::setFeedMode(feed_mode_t mode) {
  feed_mode_t oldMode = getFeedMode();
  MachineAdapter::setFeedMode(mode);

  if (oldMode != mode)
    switch (mode) {
    case INVERSE_TIME:         *stream << "G93\n"; break;
    case UNITS_PER_MINUTE:     *stream << "G94\n"; break;
    case UNITS_PER_REVOLUTION: *stream << "G95\n"; break;
    default: THROW("Feed mode must be one of INVERSE_TIME, "
                   "UNITS_PER_MIN or UNITS_PER_REV");
    }
}


void GCodeMachine::setSpeed(double speed) {
  double oldSpeed = getSpeed();
  MachineAdapter::setSpeed(speed);

  if (oldSpeed != speed) {
    beginLine();

    if (0 < speed) *stream << "M3 S" << dtos(speed, false) << '\n';
    else if (speed < 0) *stream << "M4 S" << dtos(-speed, false) << '\n';
    else *stream << "M5\n";
  }
}


void GCodeMachine::setSpinMode(spin_mode_t mode, double max) {
  double oldMax = 0;
  spin_mode_t oldMode = getSpinMode(&oldMax);

  MachineAdapter::setSpinMode(mode, max);

  if (oldMode != mode || (oldMax != max && mode == CONSTANT_SURFACE_SPEED)) {
    beginLine();

    switch (mode) {
    case REVOLUTIONS_PER_MINUTE: *stream << "G97\n"; break;
    case CONSTANT_SURFACE_SPEED:
      *stream << "G96 S" << getSpeed(); // Must output speed with G96
      if (max) *stream << " D" << dtos(max, false);
      *stream << '\n';
      break;
    }
  }
}


void GCodeMachine::setPathMode(path_mode_t mode, double motionBlending,
                               double naiveCAM) {
  MachineAdapter::setPathMode(mode, motionBlending, naiveCAM);

  beginLine();

  switch (mode) {
  case EXACT_PATH_MODE: *stream << "G61\n";   return;
  case EXACT_STOP_MODE: *stream << "G61.1\n"; return;
  case CONTINUOUS_MODE: {
    *stream << "G64";
    bool imperial = units == Units::IMPERIAL;
    if (0 < motionBlending) *stream << " P" << dtos(motionBlending, imperial);
    if (0 < naiveCAM) *stream << " Q" << dtos(naiveCAM, imperial);
    *stream << '\n';
    return;
  }
  }

  THROW("Invalid path mode " << mode);
}


void GCodeMachine::changeTool(unsigned tool) {
  unsigned currentTool = get(TOOL_NUMBER, NO_UNITS);

  MachineAdapter::changeTool(tool);

  if (tool != currentTool) {
    beginLine();
    *stream << "M6 T" << tool << '\n';
  }
}


void GCodeMachine::input(port_t port, input_mode_t mode, double timeout) {
  MachineAdapter::input(port, mode, timeout);
  // TODO Output GCode for machine.input()
}


void GCodeMachine::seek(port_t port, bool active, bool error) {
  MachineAdapter::seek(port, active, error);
  // TODO Output GCode for machine.seek()
}


void GCodeMachine::output(port_t port, double value) {
  if (value)
    switch (port) {
    case FLOOD: *stream << "M7\n"; return;
    case MIST:  *stream << "M8\n"; return;
    default: break;
    }

  else
    switch (port) {
    case FLOOD: *stream << "M9\n"; return; // M9 turns off both mist and flood
    case MIST: return;
    default: break;
    }

  return MachineAdapter::output(port, value);
}


void GCodeMachine::setPosition(const Axes &position) {
  MachineAdapter::setPosition(position);
  this->position = getTransforms().transform(position);
}


void GCodeMachine::dwell(double seconds) {
  beginLine();
  *stream << "G4 P" << dtos(seconds, false) << '\n';
  MachineAdapter::dwell(seconds);
}


bool is_near(double x, double y) {
  return Math::nextDown(y) <= x && x <= Math::nextUp(y);
}


void GCodeMachine::move(const Axes &_target, int axes, bool rapid) {
  MachineAdapter::move(_target, axes, rapid);

  bool first = true;
  bool imperial = units == Units::IMPERIAL;
  Axes target = getTransforms().transform(_target);

  for (const char *axis = Axes::AXES; *axis; axis++) {
    int axisVT = getVarType(*axis);
    bool wasSeen = axisSeen & axisVT;

    if (!wasSeen && !(axisVT & axes)) continue;

    axisSeen |= axisVT;

    string last = dtos(position.get(*axis), imperial).toString();
    string next = dtos(target.get(*axis), imperial).toString();

    // Always output axis the first time
    if (wasSeen && last == next) continue;

    if (first) {
      beginLine();
      *stream << (rapid ? "G0" : "G1");
      first = false;
    }

    *stream << ' ' << *axis << next;
  }

  if (!first) {
    *stream << '\n';
    position = target;
  }
}


void GCodeMachine::arc(const Vector3D &_offset, const Vector3D &_target,
                       double angle, plane_t _plane) {
  Plane plane(_plane);
  Transform &t = getTransforms().get(XYZ).top();
  Vector3D offset = t.transform(_offset + getPosition(XYZ)) - position.getXYZ();
  Vector3D target = t.transform(_target);

  MachineAdapter::arc(_offset, _target, angle, _plane);

  if (!angle) return;

  // Angle
  bool clockwise = 0 < angle;
  unsigned p = 0;
  if (2 * M_PI < fabs(angle)) {
    p = (unsigned)floor(fabs(angle) / (2 * M_PI));
    angle = fmod(angle, 2 * M_PI);
  }

  // Print it
  beginLine();
  *stream << 'G' << (clockwise ? 2 : 3);
  if (p) *stream << " P" << p;

  // Target
  bool imperial = units == Units::IMPERIAL;
  for (unsigned axis = 0; axis < 3; axis++) {
    int axisVT = getVarType(Axes::AXES[axis]);
    bool wasSeen = axisSeen & axisVT;

    string last = dtos(position[axis], imperial).toString();
    string next = dtos(target[axis], imperial).toString();

    // Always output axis the first time
    if (wasSeen && last == next) continue;
    axisSeen |= axisVT;

    *stream << ' ' << Axes::AXES[axis] << next;
  }

  // Offsets
  const unsigned *axisIndex = plane.getAxisIndex();
  const char *offsets = plane.getOffsets();
  for (unsigned axis = 0; axis < 2; axis++) {
    double off = offset[axisIndex[axis]];
    if (off) *stream << ' ' << offsets[axis] << off;
  }

  *stream << '\n';
  position.setXYZ(target);
}


void GCodeMachine::pause(pause_t type) {
  MachineAdapter::pause(type);

  string code;
  switch (type) {
  case PAUSE_PROGRAM:       code = "M0";  break;
  case PAUSE_OPTIONAL:      code = "M1";  break;
  case PAUSE_PALLET_CHANGE: code = "M60"; break;
  }

  beginLine();
  *stream << code << '\n';
}


void GCodeMachine::comment(const string &s) const {
  MachineAdapter::comment(s);

  vector<string> lines;
  String::tokenize(s, lines, "\n\r", true);

  for (unsigned i = 0; i < lines.size(); i++)
    *stream << "(" << lines[i] << ")\n";
}


void GCodeMachine::message(const string &s) {
  MachineAdapter::message(s);

  *stream << "(MSG," << String::escapeC(s) << ")\n";
}
