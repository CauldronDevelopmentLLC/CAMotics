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

#include "Planner.h"
#include "Runner.h"

using namespace cb;
using namespace std;
using namespace GCode;


Planner::Planner() : ControllerImpl(pipeline), started(false) {
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


Axes Planner::getPosition() const {return getAbsolutePosition();}


void Planner::setPosition(const Axes &position) {
  setAbsolutePosition(position, METRIC);
  pipeline.setPosition(position);
}


void Planner::setConfig(const PlannerConfig &config) {
  setUnits(config.defaultUnits);
  unitAdapter.setTargetUnits(config.outputUnits);
  linearizer.setMaxArcError(config.maxArcError);
  planner.setConfig(config);
}


bool Planner::isRunning() const {return !runners.empty() || !planner.isDone();}


void Planner::load(const InputSource &source, const PlannerConfig &config) {
  runners.push_back(new Runner(*this, source, config));
}


bool Planner::hasMore() {
  while (true) {
    if (!runners.empty() && !runners.front()->hasMore()) {
      runners.pop_front();
      pipeline.end();
      started = false;
    }

    if (planner.hasMove()) return true;

    if (ControllerImpl::isSynchronizing() || runners.empty()) return false;

    Runner &runner = *runners.front();

    if (!started) {
      setConfig(runner.getConfig());
      pipeline.start();
      started = true;
    }

    // Push a line of GCode to the planner
    runner.step();
  }
}


uint64_t Planner::next(JSON::Sink &sink) {return planner.next(sink);}
void Planner::setActive(uint64_t id) {planner.setActive(id);}


void Planner::stop() {
  if (ControllerImpl::isSynchronizing())
    ControllerImpl::synchronize(0); // 0 indicates probe was unsuccessful
  planner.stop();
  runners.clear();
}


void Planner::restart(uint64_t id, const Axes &position) {
  if (!planner.restart(id, position)) setPosition(position);
}


double Planner::resolve(const string &name, Units units) const {
  return resolver.isNull() ? 0 : resolver->get(name, units);
}
