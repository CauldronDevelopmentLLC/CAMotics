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

#include "LineCommand.h"

#include "PlannerConfig.h"
#include "SCurve.h"

#include <gcode/Axes.h>

#include <cbang/json/Sink.h>
#include <cbang/log/Logger.h>
#include <cbang/geom/Segment.h>

#include <limits>
#include <cmath>

using namespace GCode;
using namespace cb;
using namespace std;


LineCommand::LineCommand(const Axes &start, const Axes &end, double feed,
                         bool rapid, bool seeking, bool first,
                         const PlannerConfig &config) :
  feed(feed), start(start), target(end), rapid(rapid), seeking(seeking),
  first(first) {computeLimits(config);}


double LineCommand::getTime() const {
  double mins = 0;
  for (int i = 0; i < 7; i++) mins += times[i];
  return mins * 60;
}


bool LineCommand::canBlend() const {
  // Not compatible for blend if any of the ABC or UVW axes are moving
  for (unsigned i = 3; i < 9; i++)
    if (unit[i]) return false;

  return !seeking;
}


bool LineCommand::canMerge() const {return !seeking;}


namespace {
  double mergeError(const Axes &A, const Axes &B, const Axes &C) {
    return Segment<9, double>(A, C).distance(B);
  }
}


bool LineCommand::merge(const LineCommand &lc, const PlannerConfig &config,
                        double speed) {
  // Check if merge is possible
  if (rapid != lc.rapid || !canMerge() || !lc.canMerge()) return false;

  if (config.maxMergeLength < lc.length || config.maxMergeLength < length)
    return false;

  // Check if too many merges have already been made
  if (63 < merged.size()) return false;

  // Compute error if lines were merged
  double maxError =
    config.pathMode == PathMode::CONTINUOUS_MODE ? config.maxMergeError : 0;
  double error = mergeError(start, target, lc.target);
  if (maxError < error) return false;

  // Check errors of previously merged points
  for (unsigned i = 0; i < merged.size(); i++)
    if (maxError < mergeError(start, merged[i], lc.target))
      return false;

  LOG_DEBUG(3, "Merging moves length=" << length << " + " << lc.length
            << " target=" << target << " -> " << lc.target
            << " start=" << start << " error=" << error);

  // Handle speed
  if (!std::isnan(speed)) speeds.push_back(Speed(length, speed));

  // Merge feed rates
  if (feed != lc.feed)
    feed = (feed * length + lc.feed * lc.length) / (length + lc.length);

  // Merge
  merged.push_back(target);
  target = lc.target;
  computeLimits(config);

  return true;
}


void LineCommand::restart(const Axes &position, const PlannerConfig &config) {
  // Drop speeds that have been passed and adjust offsets of remaining speeds
  double newLength = (target - position).length();
  std::vector<Speed> newSpeeds;

  for (unsigned i = 0; i < speeds.size(); i++)
    if (newLength <= speeds[i].offset)
      newSpeeds.push_back(Speed(speeds[i].offset - newLength, speeds[i].speed));

  speeds = newSpeeds;

  // Recompute limits from new starting point
  start.setFrom(position); // Copies non-NaN values
  computeLimits(config);
}


void LineCommand::insert(JSON::Sink &sink) const {
  sink.insertDict("target", true);
  for (unsigned i = 0; i < target.getSize(); i++)
    if (target[i] != start[i])
      sink.insert(Axes::toAxisName(i, true), target[i]);
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

  if (speeds.size()) {
    sink.insertList("speeds");
    for (unsigned i = 0; i < speeds.size(); i++) {
      sink.appendList(true);
      sink.append(speeds[i].offset);
      sink.append(speeds[i].speed);
      sink.endList();
    }
    sink.endList();
  }
}


void LineCommand::write(MachineInterface &machine) const {
  // Get moved axes
  int axes = 0;

  for (unsigned i = 0; i < target.getSize(); i++)
    if (target[i] != start[i])
      axes |= MachineEnum::getVarType(Axes::toAxis(i));

  // Feed
  if (!rapid) machine.setFeed(feed);

  // Speeds
  double time = getTime();
  double offset = 0;
  for (unsigned i = 0; i < speeds.size(); i++) {
    const Speed &s = speeds[i];

    // Approximate move time
    double delta = (s.offset - offset) / length * time;
    offset += s.offset;

    machine.move(start + unit * s.offset, axes, rapid, delta);
    machine.setSpeed(s.speed);
  }

  if (offset) time = (length - offset) / length * time;
  machine.move(target, axes, rapid, time);
}


void LineCommand::cut(double length, vector<Speed> &speeds, double offset,
                      bool fromEnd) {
  this->length -= length;
  if (fromEnd) target = start + unit * this->length;
  else start = start + unit * length;

  // Get removed offset speeds
  vector<Speed> newSpeeds;
  for (unsigned i = 0; i < this->speeds.size(); i++) {
    const auto &s = this->speeds[i];

    if (fromEnd && this->length < s.offset)
      speeds.push_back(Speed(offset + s.offset - this->length, s.speed));
    else if (!fromEnd && s.offset < length)
      speeds.push_back(Speed(offset + s.offset, s.speed));
    else if (fromEnd) newSpeeds.push_back(s);
    else newSpeeds.push_back(Speed(s.offset - length, s.speed));
  }

  this->speeds = newSpeeds;
}


void LineCommand::computeLimits(const PlannerConfig &config) {
  // Reset velocities, accel and jerk
  entryVel = exitVel = maxVel = feed;
  maxAccel = numeric_limits<double>::max();
  maxJerk = numeric_limits<double>::max();

  // Handle path mode
  if (config.pathMode == MachineEnum::EXACT_PATH_MODE ||
      config.pathMode == MachineEnum::EXACT_STOP_MODE)
    entryVel = exitVel = 0;

  // Compute delta vector and length
  Axes delta = target - start;
  length = delta.length();

  if (!length) return; // Ignore null move
  if (!isfinite(length)) THROW("Invalid length " << length);

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

  LOG_DEBUG(3, "Limits computed length=" << length << " maxVel=" << maxVel
            << " maxJerk=" << maxJerk << " maxAccel=" << maxAccel);
}
