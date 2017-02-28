/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/net/URI.h>
#include <cbang/log/Logger.h>

#include <limits>

using namespace cb;
using namespace std;
using namespace CAMotics;


namespace {
  struct dtos {
    double x;
    dtos(double x) : x(x) {
      if (Math::isnan(x))
        THROW("Numerical error in GCode stream:  NaN, caused by a divide by "
              "zero or other math error.");

      if (Math::isinf(x))
        THROW("Numerical error in GCode stream: Infiniate value");
    }
  };


  inline ostream &operator<<(ostream &stream, const dtos &d) {
    return stream << String(d.x);
  }
}


void GCodeMachine::beginLine() {
#define DIGIT_CHARS        "1234567890"
#define LOWER_CHARS        "abcdefghijklmnopqrstuvwxyz"
#define UPPER_CHARS        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHA_CHARS        LOWER_CHARS UPPER_CHARS
#define ALPHANUMERIC_CHARS ALPHA_CHARS DIGIT_CHARS
#define UNESCAPED ALPHANUMERIC_CHARS "-_.!~*/"

  const FileLocation &newLoc = getLocation().getStart();
  const string &filename = newLoc.getFilename();

  if (filename != location.getFilename()) {
    stream << "(File: '" << URI::encode(filename, UNESCAPED) << "')\n";
    location.setFilename(filename);
  }

  if (newLoc.getLine() != location.getLine()) {
    stream << "N" << newLoc.getLine() << ' ';
    location.setLine(newLoc.getLine());
  }
}


void GCodeMachine::start() {
  stream << (units == Units::METRIC ? "G21" : "G20") << "\n";
  // TODO set other GCode state
}


void GCodeMachine::end() {
  // Probably should be part of the machine description.
  stream << "M2\n";
}


void GCodeMachine::setFeed(double feed, feed_mode_t mode) {
  feed_mode_t oldMode;
  double oldFeed = getFeed(&oldMode);

  MachineAdapter::setFeed(feed, mode);

  if (oldMode != mode)
    switch (mode) {
    case INVERSE_TIME:  stream << "G93\n"; break;
    case MM_PER_MINUTE: stream << "G94\n"; break;
    case MM_PER_REVOLUTION: stream << "G95\n"; break;
    default: THROW("Feed mode must be one of INVERSE_TIME, "
                   "UNITS_PER_MIN or UNITS_PER_REV");
    }

  if (feed != oldFeed) {
    beginLine();
    stream << "F" << dtos(feed) << '\n';
  }
}


void GCodeMachine::setSpeed(double speed, spin_mode_t mode, double max) {
  spin_mode_t oldMode;
  double oldSpeed = getSpeed(&oldMode);

  MachineAdapter::setSpeed(speed, mode, max);

  if (oldMode != mode)
    beginLine();

    switch (mode) {
    case REVOLUTIONS_PER_MINUTE: stream << "G97\n"; break;
    case CONSTANT_SURFACE_SPEED:
      stream << "G96 S" << dtos(fabs(speed));
      if (max) stream << " D" << dtos(max);
      stream << '\n';
      break;
    }

  if (oldSpeed != speed) {
    beginLine();

    if (0 < speed) stream << "M3 S" << dtos(speed) << '\n';
    else if (speed < 0) stream << "M4 S" << dtos(-speed) << '\n';
    else stream << "M5\n";
  }
}


void GCodeMachine::setTool(unsigned tool) {
  int oldTool = getTool();

  MachineAdapter::setTool(tool);

  if (oldTool != (int)tool) {
    beginLine();
    stream << "M6 T" << tool << '\n';
  }
}


int GCodeMachine::findPort(port_t type, unsigned index) {
  switch (type) {
  case MIST_COOLANT: if (index == 0) return 0;
  case FLOOD_COOLANT: if (index == 0) return 1;
  case PROBE: if (index == 0) return 2;
  case ANALOG: if (index < 4) return 3 + index;
  case DIGITAL: if (index < 4) return 7 + index;
  }

  return MachineAdapter::findPort(type, index);
}


double GCodeMachine::input(unsigned port, input_mode_t mode, double timeout,
                           bool error) {
  switch (port) {
  case 0: return mistCoolant;
  case 1: return floodCoolant;
  case 2: return 0; // probe
  case 3: case 4: case 5: case 6: return 0; // analog
  case 7: case 8: case 9: case 10: return 0; // digital
  }

  return MachineAdapter::input(port, mode, timeout, error);
}


void GCodeMachine::output(unsigned port, double value, bool sync) {
  switch (port) {
  case 0: mistCoolant = value; return;
  case 1: floodCoolant = value; return;
  case 2: return; // probe
  case 3: case 4: case 5: case 6: return; // analog
  case 7: case 8: case 9: case 10: return; // digital
  }

  return MachineAdapter::output(port, value, sync);
}


void GCodeMachine::dwell(double seconds) {
  beginLine();
  stream << "G4 P" << dtos(seconds) << '\n';
  MachineAdapter::dwell(seconds);
}


bool is_near(double x, double y) {
  return y - numeric_limits<double>::epsilon() <= x &&
    x <= y + numeric_limits<double>::epsilon();
}


void GCodeMachine::move(const Axes &axes, bool rapid) {
  bool first = true;
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (!is_near(position.get(*axis), axes.get(*axis))) {
      if (first) {
        beginLine();
        stream << 'G' << (rapid ? '0' : '1');
        first = false;
      }

      stream << ' ' << *axis << dtos(axes.get(*axis));
    }

  if (!first) {
    stream << '\n';
    position = axes;

    MachineAdapter::move(position, rapid);
  }
}


void GCodeMachine::pause(bool optional) {
  beginLine();
  stream << (optional ? "M1" : "M0") << '\n';
  MachineAdapter::pause(optional);
}
