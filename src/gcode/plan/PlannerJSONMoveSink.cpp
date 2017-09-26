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

#include "PlannerJSONMoveSink.h"

using namespace GCode;


void PlannerJSONMoveSink::start() {
  sink.beginList();
}


void PlannerJSONMoveSink::end() {
  sink.endList();
}


void PlannerJSONMoveSink::setSpeed(double speed) {}
void PlannerJSONMoveSink::setTool(unsigned tool) {}
void PlannerJSONMoveSink::dwell(double seconds) {}
void PlannerJSONMoveSink::pause(bool optional) {}


void PlannerJSONMoveSink::move(double time, double velocity,
                               const Axes &position) {
  sink.appendList(true);
  sink.append(time * 60000); // mS
  sink.append(velocity);
  for (unsigned i = 0; i < 4; i++) sink.append(position[i]);
  sink.endList();
}
