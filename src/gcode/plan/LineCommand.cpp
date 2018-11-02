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
#include "SCurve.h"

#include <gcode/Axes.h>

#include <cbang/json/Sink.h>

#include <limits>
#include <cmath>

using namespace GCode;
using namespace cb;
using namespace std;


LineCommand::LineCommand(uint64_t id, const Axes &start, const Axes &end,
                         double feed, bool rapid, bool seeking, bool first,
                         const PlannerConfig &config) :
  PlannerCommand(id), feed(feed), start(start), target(end), length(0),
  entryVel(0), exitVel(0), deltaV(0), maxVel(0), maxAccel(0), maxJerk(0),
  rapid(rapid), seeking(seeking), first(first), error(0) {

  // Zero times
  for (int i = 0; i < 7; i++) times[i] = 0;
  computeLimits(config);
}


bool LineCommand::merge(const LineCommand &lc, const PlannerConfig &config,
                        double speed) {
  // Check if moves are compatible
  if (lc.rapid != rapid || lc.seeking != seeking || lc.first != first)
    return false;

  // Compute angle
  const double theta = unit.angleBetween(lc.unit);
  const double a = length;
  const double b = lc.length;

  if (theta && a && b) {
    // Compute error if moves are merged
    const double c = sqrt(a * a + b * b - 2 * a * b * cos(theta));
    const double error = a * b * sin(theta) / c;

    if (config.maxMergeError < error + this->error) return false;

    if (config.maxColinearAngle < theta) {
      // Check if move is too long for merge
      if (config.maxMergeLength < lc.length || config.maxMergeLength < length)
        return false;

      // Check move time
      double mins = 0;
      for (int i = 0; i < 7; i++) mins += times[i];
      if (config.minMoveSecs <= mins * 60) return false;
    }

    // Accumulate errors
    this->error += error;
  }

  // Handle speed
  if (!std::isnan(speed)) speeds.push_back(Speed(length, speed));

  // Merge
  target = lc.target;
  computeLimits(config);

  return true;
}


void LineCommand::restart(const Axes &position, const PlannerConfig &config) {
  start = position;
  computeLimits(config);
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

  if (rapid) sink.insertBoolean("rapid", true);
  if (seeking) sink.insertBoolean("seeking", true);
  if (first) sink.insertBoolean("first", true);

  sink.insertList("times", true);
  for (unsigned i = 0; i < 7; i++)
    sink.append(times[i] * 60000); // ms
  sink.endList();

  vector<double> offsetTimes;
  computeOffsetTimes(offsetTimes);

  if (speeds.size()) {
    sink.insertList("speeds");
    for (unsigned i = 0; i < speeds.size(); i++) {
      sink.appendList(true);
      sink.append(offsetTimes[i] * 60000); // ms
      sink.append(speeds[i].offset);
      sink.append(speeds[i].speed);
      sink.endList();
    }
    sink.endList();
  }
}


void LineCommand::computeOffsetTimes(vector<double> &offsetTimes) const {
  double segDist[7];
  double segJerk[7];
  double segAccel[7];
  double segVel[7];

  // Jerks
  segJerk[0] = segJerk[6] = maxJerk;
  segJerk[2] = segJerk[4] = -maxJerk;
  segJerk[1] = segJerk[3] = segJerk[5] = 0;

  // Accels
  segAccel[1] = segAccel[2] = maxJerk * times[0];
  segAccel[5] = segAccel[6] = -maxJerk * times[4];
  segAccel[0] = segAccel[3] = segAccel[4] = 0;

  // Compute velocities and distances for each segment
  double v = entryVel;
  for (unsigned i = 0; i < 7; i++) {
    segVel[i] = v;
    segDist[i] = SCurve::distance(times[i], segVel[i], segAccel[i], segJerk[i]);
    v += SCurve::velocity(times[i], segAccel[i], segJerk[i]);
  }

  // Compute speed offset times
  offsetTimes.resize(speeds.size());
  for (unsigned i = 0; i < speeds.size(); i++) {
    double offset = speeds[i].offset;

    // Find the segment the offset lands in
    double t = 0;
    double d = 0;
    for (unsigned j = 0; j < 7; j++) {
      if (offset <= d + segDist[j]) {
        offsetTimes[i] = t + SCurve::timeAtDistance
          (offset - d, segVel[j], segAccel[j], segJerk[j], times[j]);
        break;
      }

      t += times[j];
      d += segDist[j];
      if (j == 6) offsetTimes[i] = t;
    }
  }
}


void LineCommand::computeLimits(const PlannerConfig &config) {
  // Reset velocities, accel and jerk
  entryVel = exitVel = maxVel = feed;
  maxAccel = numeric_limits<double>::max();
  maxJerk = numeric_limits<double>::max();

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
