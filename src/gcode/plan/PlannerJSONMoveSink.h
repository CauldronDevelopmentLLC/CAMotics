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

#pragma once

#include "PlannerSink.h"

#include <cbang/json/Sink.h>


namespace GCode {
  class PlannerJSONMoveSink : public PlannerSink {
    cb::JSON::Sink &sink;

  public:
    PlannerJSONMoveSink(cb::JSON::Sink &sink) : sink(sink) {}

    // From PlannerSink
    void start();
    void end();

    void setSpeed(double speed);
    void setTool(unsigned tool);

    void dwell(double seconds);
    void pause(bool optional);

    void move(double time, double velocity, const Axes &position);
  };
}
