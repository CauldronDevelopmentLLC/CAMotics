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


Planner::Planner() : ControllerImpl(pipeline) {
  pipeline.add(SmartPointer<MachineUnitAdapter>::Phony(&unitAdapter));
  pipeline.add(SmartPointer<MachineLinearizer>::Phony(&linearizer));
  pipeline.add(SmartPointer<LinePlanner>::Phony(&planner));
}


void Planner::setConfig(const PlannerConfig &config) {
  unitAdapter.setUnits(config.defaultUnits);
  unitAdapter.setTargetUnits(config.outputUnits);
  linearizer.setMaxArcError(config.maxArcError);
  setAbsolutePosition(config.start);
  pipeline.setPosition(config.start);
  planner.setConfig(config);
}


bool Planner::isRunning() const {return !runners.empty() || !planner.isDone();}


void Planner::overrideSync() {
  if (ControllerImpl::isSynchronizing())
    ControllerImpl::synchronize(ControllerImpl::getAbsolutePosition());
}


void Planner::load(const InputSource &source, const PlannerConfig &config) {
  runners.push_back(new Runner(*this, source, config));
}


bool Planner::hasMore() {
  while (true) {
    if (planner.hasMove()) return true;
    if (ControllerImpl::isSynchronizing() || runners.empty()) return false;

    if (!runners.front()->hasStarted()) {
      setConfig(runners.front()->getConfig());
      pipeline.start();
    }

    // Push a line of GCode to the planner
    if (!runners.front()->next()) {
      runners.pop_front();
      pipeline.end();
    }
  }
}


void Planner::next(JSON::Sink &sink) {planner.next(sink);}
void Planner::setActive(uint64_t id) {planner.setActive(id);}


void Planner::restart(uint64_t id, const Axes &position) {
  if (ControllerImpl::isSynchronizing()) ControllerImpl::synchronize(position);
  if (!planner.restart(id, position)) setAbsolutePosition(position);
}


double Planner::get(const string &name) const {
  if (ControllerImpl::has(name)) return ControllerImpl::get(name);
  return resolver.isNull() ? 0 : resolver->get(name);
}
