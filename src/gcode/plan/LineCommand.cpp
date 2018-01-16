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


LineCommand::LineCommand(uint64_t id, const Vector4D &start,
                         const Vector4D &end, double feed,
                         const PlannerConfig &config) :
  PlannerCommand(id), length(0), entryVel(feed), exitVel(feed), deltaV(0),
  maxVel(feed), maxAccel(numeric_limits<double>::max()),
  maxJerk(numeric_limits<double>::max()) {

  // Zero times
  for (int i = 0; i < 7; i++) times[i] = 0;

  for (int i = 0; i < 4; i++)
    target[i] = end[i];

  computeLimits(start, config);
}


void LineCommand::restart(const Axes &position, const PlannerConfig &config) {
  Vector4D start;
  for (int i = 0; i < 4; i++) start[i] = position[i];

  computeLimits(start, config);
}


void LineCommand::insert(JSON::Sink &sink) const {
  sink.insertDict("target", true);
  for (unsigned i = 0; i < target.getSize(); i++)
    sink.insert(Axes::toAxisName(i, true), target[i]);
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


void LineCommand::computeLimits(const Vector4D &start,
                                const PlannerConfig &config) {
  // Compute axis and lengths
  Vector4D delta = target - start;
  length = delta.length();

  if (!length) return; // Null move
  if (!isfinite(length)) THROWS("Invalid length");

  unit = delta / length;

  // Apply axis velocity limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && config.maxVel[i] && isfinite(config.maxVel[i])) {
      double v = fabs(config.maxVel[i] / unit[i]);
      if (v < maxVel) maxVel = v;
    }

  // Apply axis jerk limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && config.maxJerk[i] && isfinite(config.maxJerk[i])) {
      double j = fabs(config.maxJerk[i] / unit[i]);
      if (j < maxJerk) maxJerk = j;
    }

  // Apply axis acceleration limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && config.maxAccel[i] && isfinite(config.maxAccel[i])) {
      double a = fabs(config.maxAccel[i] / unit[i]);
      if (a < maxAccel) maxAccel = a;
    }

  // Limit entry & exit velocities
  if (maxVel < entryVel) entryVel = maxVel;
  if (maxVel < exitVel) exitVel = maxVel;
}
