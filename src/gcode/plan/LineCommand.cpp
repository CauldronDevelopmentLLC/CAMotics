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

#include "LineCommand.h"

#include "PlannerConfig.h"

#include <gcode/Axes.h>

#include <cbang/json/Sink.h>

#include <limits>

using namespace GCode;
using namespace cb;
using namespace std;


LineCommand::LineCommand(uint64_t id, const Axes &start,
                         const Axes &end, double feed, bool seeking,
                         const PlannerConfig &config) :
  PlannerCommand(id), target(end), length(0), entryVel(feed), exitVel(feed),
  deltaV(0), maxVel(feed), maxAccel(numeric_limits<double>::max()),
  maxJerk(numeric_limits<double>::max()), seeking(seeking) {

  // Zero times
  for (int i = 0; i < 7; i++) times[i] = 0;
  computeLimits(start, config);
}


void LineCommand::restart(const Axes &position, const PlannerConfig &config) {
  computeLimits(position, config);
}


void LineCommand::insert(JSON::Sink &sink) const {
  sink.insertDict("target", true);
  for (unsigned i = 0; i < target.getSize(); i++)
    if (unit[i]) sink.insert(Axes::toAxisName(i, true), target[i]);
  sink.endDict();

  sink.insert("entry-vel", entryVel);
  sink.insert("exit-vel", exitVel);
  sink.insert("max-vel", maxVel);
  sink.insert("max-accel", maxAccel);
  sink.insert("max-jerk", maxJerk);

  sink.insertList("times", true);
  for (unsigned i = 0; i < 7; i++)
    sink.append(times[i] * 60000); // ms
  sink.endList();
}


void LineCommand::computeLimits(const Axes &start,
                                const PlannerConfig &config) {
  // Compute delta vector and length
  Axes delta = target - start;
  length = delta.length();

  if (!length) return; // Ignore null move
  if (!isfinite(length)) THROWS("Invalid length " << length);

  // Compute unit vector
  unit = delta / length;

  // Apply limits
  for (unsigned axis = 0; axis < Axes::getSize(); axis++) {
    if (!unit[axis]) continue;

    // Velocity limit
    if (config.maxVel[axis] && isfinite(config.maxVel[axis])) {
      double v = fabs(config.maxVel[axis] / unit[axis]);
      if (v < maxVel) maxVel = v;
    }

    // Jerk limit
    if (config.maxJerk[axis] && isfinite(config.maxJerk[axis])) {
      double j = fabs(config.maxJerk[axis] / unit[axis]);
      if (j < maxJerk) maxJerk = j;
    }

    // Acceleration limit
    if (config.maxAccel[axis] && isfinite(config.maxAccel[axis])) {
      double a = fabs(config.maxAccel[axis] / unit[axis]);
      if (a < maxAccel) maxAccel = a;
    }
  }

  // Seeking
  if (seeking) exitVel = 0;

  // Limit entry & exit velocities
  if (maxVel < entryVel) entryVel = maxVel;
  if (maxVel < exitVel) exitVel = maxVel;
}
