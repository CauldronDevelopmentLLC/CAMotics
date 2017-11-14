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

#include "PlannerConfig.h"
#include "LinePlanner.h"

#include <gcode/Controller.h>
#include <gcode/Runner.h>
#include <gcode/machine/MachinePipeline.h>


namespace cb {namespace JSON {class Sink;}}


namespace GCode {
  class Runner;

  class Planner {
    const PlannerConfig &config;

    MachinePipeline pipeline;
    cb::SmartPointer<Controller> controller;
    LinePlanner planner;

    std::string gcode;
    cb::SmartPointer<Runner> runner;

  public:
    Planner(const PlannerConfig &config);

    bool isRunning() const;

    void mdi(const std::string &gcode);
    void load(const cb::InputSource &source);

    bool hasMore();
    void next(cb::JSON::Sink &sink);
    void release(uint64_t line);
    void restart(uint64_t line, double length);
  };
}
