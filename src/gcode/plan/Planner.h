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
#include "PlannerConfig.h"
#include "PlanCommand.h"
#include "PlanQueue.h"

#include <gcode/machine/MachineAdapter.h>


namespace GCode {
  class Planner : public MachineAdapter {
    PlannerSink &sink;
    PlannerConfig config;

    double velocity;
    Axes position;

    cb::FileLocation location;

    PlanQueue queue;

  public:
    Planner(PlannerSink &sink, const PlannerConfig &config) :
      sink(sink), config(config), velocity(0) {}

    // From MachineInterface
    void start();
    void end();

    void setSpeed(double speed, spin_mode_t mode, double max);
    void setTool(unsigned tool);

    void dwell(double seconds);
    void move(const Axes &axes, bool rapid);
    void pause(bool optional);

  protected:
    void plan();
  };
}
