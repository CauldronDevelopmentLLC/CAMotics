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

#include "LineCommand.h"

#include <gcode/Axes.h>

#include <cbang/json/Sink.h>

#include <limits>

using namespace GCode;
using namespace cb;
using namespace std;


LineCommand::LineCommand(uint64_t line) :
  PlannerCommand(line), length(0), entryVel(numeric_limits<double>::max()),
  exitVel(numeric_limits<double>::max()), deltaV(0),
  maxVel(numeric_limits<double>::max()),
  maxAccel(numeric_limits<double>::max()),
  maxJerk(numeric_limits<double>::max()) {}


void LineCommand::restart(double length) {
  if (this->length < length) THROWS("Cannot restart from length " << length);

  setEntryVelocity(0);
  this->length -= length;
}


void LineCommand::insert(JSON::Sink &sink) {
  sink.insertDict("target", true);
  for (unsigned i = 0; i < position.getSize(); i++)
    sink.insert(Axes::toAxisName(i, true), position[i]);
  sink.endDict();

  sink.insert("exit-vel", exitVel);
  sink.insert("max-vel", maxVel);
  sink.insert("max-accel", maxAccel);
  sink.insert("max-jerk", maxJerk);

  sink.insertList("times", true);
  for (unsigned i = 0; i < 7; i++)
    sink.append(times[i] * 60000); // ms
  sink.endList();
}
