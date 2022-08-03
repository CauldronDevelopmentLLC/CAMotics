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

#include "Planner.h"
#include "GCodeRunner.h"
#include "TPLRunner.h"

#include <cbang/json/Builder.h>

using namespace cb;
using namespace std;
using namespace GCode;


Planner::Planner() : controller(pipeline) {
  class ParamResolver : public MachineAdapter {
    Planner &planner;

  public:
    ParamResolver(Planner &planner) : planner(planner) {}

    // From MachineInterface
    double get(const string &name, Units units) const {
      if (MachineAdapter::has(name)) return MachineAdapter::get(name, units);
      return planner.resolve(name, units);
    }
  };

  pipeline.add(SmartPointer<MachineUnitAdapter>::Phony(&unitAdapter));
  pipeline.add(SmartPointer<MachineLinearizer>::Phony(&linearizer));
  pipeline.add(new ParamResolver(*this));
  pipeline.add(SmartPointer<LinePlanner>::Phony(&planner));
}


void Planner::setPosition(const Axes &position) {
  pipeline.setPosition(position);
  controller.setAbsolutePosition(position, Units::METRIC);
}


void Planner::setConfig(const PlannerConfig &config) {
  controller.setUnits(config.defaultUnits);
  unitAdapter.setTargetUnits(config.outputUnits);
  pipeline.set("_max_arc_error", config.maxArcError, Units::METRIC);
  planner.setConfig(config);
}


bool Planner::isRunning() const {return !runners.empty() || !planner.isEmpty();}


void Planner::load(const InputSource &source, const PlannerConfig &config,
                   bool tpl) {
  if (tpl) {
#if !defined(CAMOTICS_NO_TPL) && (defined(HAVE_V8) || defined(HAVE_CHAKRA))
    runners.push_back(new TPLRunner(pipeline, source, config));
#else
    THROW("TPL not supported in this build");
#endif
  } else runners.push_back(new GCodeRunner(controller, source, config));
}


bool Planner::hasMore() {
  while (true) {
    if (!runners.empty() && !runners.front()->hasMore()) {
      runners.pop_front();
      pipeline.end();
      started = false;
    }

    if (planner.hasMove()) return true;
    if (isSynchronizing() || runners.empty()) return false;

    Runner &runner = *runners.front();

    if (!started) {
      setConfig(runner.getConfig());
      pipeline.start();
      started = true;
    }

    // Advance the interpreter
    runner.step();
  }
}


uint64_t Planner::next(JSON::Sink &sink) {
  if (!hasMore()) THROW("No more");
  return planner.next(sink);
}


uint64_t Planner::next(MachineInterface &machine) {
  if (!hasMore()) THROW("No more");
  return planner.next(machine);
}


SmartPointer<JSON::Value> Planner::next() {
  return JSON::Builder::build([this] (JSON::Sink &sink) {next(sink);});
}


void Planner::setActive(uint64_t id) {planner.setActive(id);}


void Planner::stop() {
  if (isSynchronizing()) synchronize(0);
  planner.stop();
  runners.clear();
  controller.stop();
  started = false;
}


void Planner::restart(uint64_t id, const Axes &position) {
  if (!planner.restart(id, position)) setPosition(position);
  if (isSynchronizing()) synchronize(1);
}


double Planner::resolve(const string &name, Units units) const {
  return resolver.isNull() ? 0 : resolver->get(name, units);
}


void Planner::dumpQueue(JSON::Sink &sink) {planner.dumpQueue(sink);}
