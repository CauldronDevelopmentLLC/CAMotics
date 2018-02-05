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

#include <gcode/Axes.h>
#include <gcode/Runner.h>
#include <gcode/ControllerImpl.h>
#include <gcode/machine/MachinePipeline.h>


namespace cb {namespace JSON {class Sink;}}


namespace GCode {
  class Controller;
  class Runner;


  class NameResolver {
  public:
    virtual ~NameResolver() {}
    virtual double get(const std::string &name) = 0;
  };


  class Planner : public ControllerImpl {
    MachinePipeline pipeline;
    LinePlanner planner;

    std::string gcode;
    cb::SmartPointer<Runner> runner;

    cb::SmartPointer<NameResolver> resolver;

  public:
    Planner(const PlannerConfig &config);

    void setConfig(const PlannerConfig &config) {planner.setConfig(config);}

    void setResolver(const cb::SmartPointer<NameResolver> &resolver)
    {this->resolver = resolver;}

    bool isRunning() const;
    void overrideSync();

    void mdi(const std::string &gcode);
    void load(const cb::InputSource &source);

    bool hasMore();
    void next(cb::JSON::Sink &sink);
    void release(uint64_t id);
    void restart(uint64_t id, const Axes &position);

    // From Controller
    double get(const std::string &name) const;
  };
}
