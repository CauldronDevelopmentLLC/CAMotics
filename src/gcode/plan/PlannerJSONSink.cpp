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

#include "PlannerJSONSink.h"

#include <limits>
#include <cmath>

using namespace GCode;
using namespace cb;
using namespace std;


PlannerJSONSink::PlannerJSONSink(JSON::Sink &sink) :
  sink(sink), position(numeric_limits<double>::quiet_NaN()) {}


void PlannerJSONSink::start() {sink.beginList();}
void PlannerJSONSink::end() {sink.endList();}


void PlannerJSONSink::setSpeed(double speed) {
  sink.appendDict(true);
  sink.insert("cmd", "speed");
  sink.insert("value", speed);
  sink.endDict();
}


void PlannerJSONSink::setTool(unsigned tool) {
  sink.appendDict(true);
  sink.insert("cmd", "tool");
  sink.insert("value", tool);
  sink.endDict();
}


void PlannerJSONSink::dwell(double seconds) {
  sink.appendDict(true);
  sink.insert("cmd", "dwell");
  sink.insert("value", seconds);
  sink.endDict();
}


void PlannerJSONSink::pause(bool optional) {
  sink.appendDict(true);
  sink.insert("cmd", optional ? "optional pause" : "pause");
  sink.endDict();
}


void PlannerJSONSink::move(const Axes &position, double maxVel) {
  sink.appendDict(true);
  sink.insert("cmd", "move");

  for (unsigned i = 0; i < 6; i++)
    if (isfinite(position[i]) && position[i] != this->position[i])
      sink.insert(string(1, Axes::toAxis(i)), position[i]);

  this->position = position;

  sink.endDict();
}
