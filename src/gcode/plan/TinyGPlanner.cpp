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

#include "TinyGPlanner.h"

#include <gcode/plan/bbctrl/line.h>
#include <gcode/plan/bbctrl/exec.h>
#include <gcode/plan/bbctrl/buffer.h>

#include <cbang/Exception.h>

using namespace cb;
using namespace std;
using namespace GCode;


PlannerSink *TinyGPlanner::sink = 0;
PlannerConfig TinyGPlanner::config;


TinyGPlanner::TinyGPlanner(PlannerSink &sink, const PlannerConfig &config) {
  TinyGPlanner::sink = &sink;
  TinyGPlanner::config = config;
}


void TinyGPlanner::start() {
  mp_queue_init();
  last_vel = 0;

  real start[AXES];
  for (int i = 0; i < AXES; i++) start[i] = config.start[i];

  mp_planner_set_position(start);
  mp_exec_set_position(start);

  MachineAdapter::start();
  sink->start();
}


void TinyGPlanner::end() {
  MachineAdapter::end();

  while (true) {
    mp_buffer_t *bf = mp_queue_get_head();
    if (!bf) break;

    bf->entry_velocity = last_vel;
    last_vel = bf->exit_velocity;

    mp_exec_aline(bf);
    mp_queue_pop();
  }

  sink->end();
}


void TinyGPlanner::move(const Axes &target, bool rapid) {
  MachineAdapter::move(target, rapid);

  real _target[AXES];
  for (int i = 0; i < AXES; i++) _target[i] = target[i];

  int flags = 0;
  if (rapid) flags |= BUFFER_RAPID;

  int line = getLocation().getStart().getLine();

  mp_aline(_target, flags, SW_NONE, getFeed(), 1, line);

  while (true) {
    mp_buffer_t *bf = mp_queue_get_head();
    if (!bf || (bf->flags & BUFFER_REPLANNABLE && 4 < mp_queue_get_room()))
      break;

    bf->entry_velocity = last_vel;
    last_vel = bf->exit_velocity;

    mp_exec_aline(bf);
    mp_queue_pop();
  }
}


void mp_move(real time, real velocity, const real target[]) {
  if (!isfinite(target[0]) || !isfinite(target[1]) ||
      !isfinite(target[2]) || !isfinite(target[3]))
    THROW("Target is not finite");

  Axes position;
  for (int i = 0; i < 4; i++) position[i] = target[i];

  TinyGPlanner::getSink().move(time, velocity, position);
}


real axis_get_jerk_max(int axis) {
  return TinyGPlanner::getConfig().maxJerk[axis] / 1000000;
}


real axis_get_velocity_max(int axis) {
  return TinyGPlanner::getConfig().maxVel[axis];
}


real get_junction_deviation() {
  return TinyGPlanner::getConfig().junctionDeviation;
}


real get_junction_acceleration() {
  return TinyGPlanner::getConfig().junctionAccel;
}
