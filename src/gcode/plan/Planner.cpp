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
#include <gcode/ControllerImpl.h>
#include <gcode/machine/MachineState.h>
#include <gcode/machine/MachineLinearizer.h>
#include <gcode/machine/MachineUnitAdapter.h>

#include <cbang/json/Writer.h>
#include <cbang/io/StringInputSource.h>


using namespace cb;
using namespace std;
using namespace GCode;


Planner::Planner(const PlannerConfig &config) :
  config(config), planner(config) {

  pipeline.add(new MachineUnitAdapter(config.defaultUnits,
                                      config.outputUnits));
  pipeline.add(new MachineLinearizer(config.maxArcError));
  pipeline.add(SmartPointer<LinePlanner>::Phony(&planner));
  pipeline.add(new MachineState);

  controller = new ControllerImpl(pipeline);
}


bool Planner::isRunning() const {
  return (!runner.isNull() && !runner->isDone()) || !planner.isDone();
}


void Planner::set(const string &name, double value) {
  controller->set(name, value);
}


void Planner::mdi(const string &gcode) {
  if (isRunning()) THROW("Planner already running");
  this->gcode = gcode;
  load(StringInputSource(this->gcode));
}


void Planner::load(const cb::InputSource &source) {
  if (isRunning()) THROW("Planner already running");
  runner = new Runner(*controller, source);
  pipeline.start();
}


bool Planner::hasMore() {
  while (true) {
    if (planner.hasMove()) return true;
    if (runner.isNull() || runner->isDone()) return false;
    runner->next();
    if (runner->isDone()) pipeline.end();
  }
}


void Planner::next(JSON::Sink &sink) {planner.next(sink);}
void Planner::release(uint64_t id) {planner.release(id);}


void Planner::restart(uint64_t id, const Axes &position) {
  planner.restart(id, position);
}
