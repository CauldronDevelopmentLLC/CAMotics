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

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/net/URI.h>
#include <cbang/log/Logger.h>

#include <limits>

using namespace cb;
using namespace std;
using namespace GCode;


void Planner::start() {
  velocity = 0;
  position = config.start;

  MachineAdapter::start();
  queue.push(new PlanStartCommand());
}


void Planner::end() {
  MachineAdapter::end();
  queue.push(new PlanEndCommand());
  plan();
}


void Planner::setSpeed(double speed, spin_mode_t mode, double max) {
  if (mode == CONSTANT_SURFACE_SPEED)
    THROWS("Constant surface speed not implemented");

  double oldSpeed = getSpeed();
  MachineAdapter::setSpeed(speed, mode, max);

  if (oldSpeed != speed) queue.push(new PlanSpeedCommand(speed));
}


void Planner::setTool(unsigned tool) {
  int oldTool = getTool();
  MachineAdapter::setTool(tool);
  if (oldTool != (int)tool) queue.push(new PlanToolCommand(tool));
}


void Planner::dwell(double seconds) {
  MachineAdapter::dwell(seconds);
  queue.push(new PlanDwellCommand(seconds));
}


void Planner::pause(bool optional) {
  MachineAdapter::pause(optional);
  queue.push(new PlanPauseCommand(optional));
}


void Planner::move(const Axes &target, bool rapid) {
  MachineAdapter::move(target, rapid);

  // TODO Handle feed rate mode
  double feed = rapid ? numeric_limits<double>::max() : getFeed();
  queue.push(new PlanMoveCommand(config, position, target, feed));
  position = target;

  plan();
}


void Planner::plan() {
  while (!queue.empty()) {
    if (!queue.front()->plan()) break;
    queue.front()->write(sink);
    queue.pop();
  }
}
