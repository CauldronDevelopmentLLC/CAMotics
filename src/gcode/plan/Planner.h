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

#pragma once

#include "PlannerConfig.h"
#include "LinePlanner.h"
#include "Runner.h"

#include <gcode/Axes.h>
#include <gcode/ControllerImpl.h>
#include <gcode/machine/MachinePipeline.h>
#include <gcode/machine/MachineUnitAdapter.h>
#include <gcode/machine/MachineLinearizer.h>

#include <list>

namespace cb {namespace JSON {class Sink;}}


namespace GCode {
  class Controller;


  class NameResolver {
  public:
    virtual ~NameResolver() {}
    virtual double get(const std::string &name, Units units) = 0;
  };


  class Planner {
    MachinePipeline pipeline;
    ControllerImpl controller;
    MachineUnitAdapter unitAdapter;
    MachineLinearizer linearizer;
    LinePlanner planner;

    std::list<cb::SmartPointer<Runner> > runners;
    cb::SmartPointer<NameResolver> resolver;

    bool started = false;

  public:
    Planner();

    double getTime() const {return planner.getTime();}
    double getDistance() const {return planner.getDistance();}

    void setPosition(const Axes &position);

    void setConfig(const PlannerConfig &config);

    void setResolver(const cb::SmartPointer<NameResolver> &resolver)
    {this->resolver = resolver;}

    bool isRunning() const;
    bool isSynchronizing() const {return controller.isSynchronizing();}
    void synchronize(double result) {controller.synchronize(result);}

    void load(const cb::InputSource &source, const PlannerConfig &config,
              bool tpl);

    bool hasMore();
    uint64_t next(cb::JSON::Sink &sink);
    uint64_t next(MachineInterface &machine);
    cb::SmartPointer<cb::JSON::Value> next();
    void setActive(uint64_t id);
    void stop();
    void restart(uint64_t id, const Axes &position);
    double resolve(const std::string &name, Units units) const;
    void dumpQueue(cb::JSON::Sink &sink);
  };
}
