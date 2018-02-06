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

#include <gcode/Runner.h>
#include <gcode/machine/MachineState.h>
#include <gcode/machine/MachineLinearizer.h>
#include <gcode/machine/MachineUnitAdapter.h>


using namespace cb;
using namespace std;
using namespace GCode;


Planner::Planner(const PlannerConfig &config) :
  ControllerImpl(pipeline), planner(config) {

  pipeline.add(new MachineUnitAdapter(config.defaultUnits,
                                      config.outputUnits));
  pipeline.add(new MachineLinearizer(config.maxArcError));
  pipeline.add(SmartPointer<LinePlanner>::Phony(&planner));
  pipeline.add(new MachineState);
}


bool Planner::isRunning() const {return !runners.empty() || !planner.isDone();}


void Planner::overrideSync() {
  if (ControllerImpl::isSynchronizing())
    ControllerImpl::synchronize(ControllerImpl::getAbsolutePosition());
}


void Planner::load(const InputSource &source) {
  runners.push_back(new Runner(*this, source));
}


bool Planner::hasMore() {
  while (true) {
    if (planner.hasMove()) return true;
    if (ControllerImpl::isSynchronizing() || runners.empty()) return false;

    if (!runners.front()->hasStarted()) pipeline.start();

    runners.front()->next();

    if (runners.front()->hasEnded()) {
      runners.pop_back();
      pipeline.end();
    }
  }
}


void Planner::next(JSON::Sink &sink) {planner.next(sink);}
void Planner::setActive(uint64_t id) {planner.setActive(id);}


void Planner::restart(uint64_t id, const Axes &position) {
  planner.restart(id, position);
  if (ControllerImpl::isSynchronizing()) ControllerImpl::synchronize(position);
}


double Planner::get(const string &name) const {
  if (ControllerImpl::has(name)) return ControllerImpl::get(name);
  return resolver.isNull() ? 0 : resolver->get(name);
}
