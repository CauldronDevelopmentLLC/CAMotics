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

#include "LinePlanner.h"

#include "SCurve.h"
#include "LineCommand.h"
#include "DwellCommand.h"
#include "PauseCommand.h"
#include "OutputCommand.h"
#include "SeekCommand.h"
#include "InputCommand.h"
#include "SetCommand.h"
#include "EndCommand.h"

#include <gcode/Helix.h>
#include <gcode/machine/Transform.h>

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/json/JSON.h>

#include <limits>
#include <algorithm>
#include <complex>

using namespace cb;
using namespace std;
using namespace GCode;


namespace {
  complex<double> cbrt(complex<double> x) {
    return x.real() < 0 ? -pow(-x, 1.0 / 3.0) : pow(x, 1.0 / 3.0);
  }
}


LinePlanner::LinePlanner() : nextID(1) {reset();}


void LinePlanner::reset() {
  lastExitVel = 0;
  seeking = false;
  firstMove = true;
  line = -1;
  speed = numeric_limits<double>::quiet_NaN();
  rapidAutoOff = false;
  time = 0;
  distance = 0;
}


void LinePlanner::setConfig(const PlannerConfig &config) {
  this->config = config;
  nextID &= (1U << config.idBits) - 1;
}


void LinePlanner::checkSoftLimits(const Axes &p) {
  for (unsigned axis = 0; axis < Axes::getSize(); axis++)
    if (isfinite(p[axis])) {
      if (isfinite(config.minSoftLimit[axis]) &&
          p[axis] < config.minSoftLimit[axis])
        THROW(Axes::toAxisName(axis) << " axis position " << p[axis]
               << "mm is less than minimum soft limit "
               << config.minSoftLimit[axis] << "mm");

      if (isfinite(config.maxSoftLimit[axis]) &&
          config.maxSoftLimit[axis] < p[axis])
        THROW(Axes::toAxisName(axis) << " axis position " << p[axis]
               << "mm is greater than maximum soft limit "
               << config.maxSoftLimit[axis] << "mm");
    }
}


bool LinePlanner::isEmpty() const {return cmds.empty();}


bool LinePlanner::hasMove() const {
  return !cmds.empty() && isFinal(cmds.front());
}


const PlannerCommand &LinePlanner::next() {
  if (!hasMove()) THROW("Planner not ready");

  PlannerCommand *cmd = cmds.front();
  out.push_back(cmds.pop_front());
  lastExitVel = cmd->getExitVelocity();

  if (!cmd->isSeeking()) {
    time += cmd->getTime();
    distance += cmd->getLength();
  }

  return *cmd;
}


uint64_t LinePlanner::next(JSON::Sink &sink) {
  const PlannerCommand &cmd = next();
  cmd.write(sink);
  return cmd.getID();
}


uint64_t LinePlanner::next(MachineInterface &machine) {
  const PlannerCommand &cmd = next();
  cmd.write(machine);
  return cmd.getID();
}


void LinePlanner::setActive(uint64_t id) {
  while (!out.empty() && idLess(out.front()->getID(), id))
    delete out.pop_front();
}


void LinePlanner::stop() {
  reset();
  nextID = 1;
  cmds.clear();
  pre.clear();
  out.clear();
}


bool LinePlanner::restart(uint64_t id, const Axes &position) {
  // Release commands before this ID
  setActive(id);

  // Reload previously output moves
  while (!out.empty()) cmds.push_front(out.pop_back());

  // Make sure we are now at the requested restart command
  if (cmds.empty() || id != cmds.front()->getID())
    THROW("Planner ID " << id << " not found.  "
           << (cmds.empty() ? String("Queue empty.") :
               SSTR("Next ID is " << cmds.front()->getID())));

  // Reset last exit velocity
  lastExitVel = 0;

  // Handle restart after seek
  PlannerCommand *cmd = cmds.front();
  if (cmd->isSeeking()) {
    // Skip rest of current move
    cmd = cmd->next;
    delete cmds.pop_front();
    // Find next move, its start position may have changed
    while (cmd && !cmd->isMove()) cmd = cmd->next;
    if (!cmd) return false; // Nothing to replan
  }

  // Replan from zero velocity
  cmd->restart(position, config);

  // Check if the restart was at the end of the command
  if (!cmd->getLength()) {
    cmds.remove(cmd);
    delete cmd;
  }

  // Replan
  for (cmd = cmds.front(); cmd; cmd = cmd->next)
    plan(cmd);

  return !cmds.empty();
}


void LinePlanner::dumpQueue(JSON::Sink &sink) {
  sink.beginList();

  for (PlannerCommand *cmd = cmds.front(); cmd; cmd = cmd->next) {
    sink.beginAppend();
    cmd->write(sink);
  }

  sink.endList();
}


void LinePlanner::start() {
  reset();
  MachineState::start();
}


void LinePlanner::end() {
  MachineState::end();
  push(new EndCommand);
}


void LinePlanner::setSpeed(double speed) {
  MachineState::setSpeed(speed);

  if (this->speed != speed) {
    this->speed = speed;
    pushSetCommand("speed", speed);
  }
}


void LinePlanner::setSpinMode(spin_mode_t mode, double max) {
  MachineState::setSpinMode(mode, max);

  switch (mode) {
  case REVOLUTIONS_PER_MINUTE: pushSetCommand("spin-mode", "rpm"); break;
  case CONSTANT_SURFACE_SPEED:
    pushSetCommand("max-rpm", max);
    pushSetCommand("spin-mode", "css");
    break;
  }
}


void LinePlanner::setPathMode(path_mode_t mode, double motionBlending,
                              double naiveCAM) {
  config.pathMode = mode;

  if (mode == CONTINUOUS_MODE) {
    // Note, value < 0 means keep previous value
    if (0 <= naiveCAM)
      config.maxMergeError =
        naiveCAM < config.minMergeError ? config.minMergeError : naiveCAM;
    if (0 <= motionBlending) config.maxBlendError = motionBlending;
  }
}


void LinePlanner::changeTool(unsigned tool) {
  MachineState::changeTool(tool);
  pushSetCommand("tool", tool);
}


void LinePlanner::input(port_t port, input_mode_t mode, double timeout) {
  MachineState::input(port, mode, timeout);
  push(new InputCommand(port, mode, timeout));
}


void LinePlanner::seek(port_t port, bool active, bool error) {
  MachineState::seek(port, active, error);
  push(new SeekCommand(port, active, error));
  seeking = true;
}



void LinePlanner::output(port_t port, double value) {
  MachineState::output(port, value);
  push(new OutputCommand(port, value));
}


void LinePlanner::dwell(double seconds) {
  MachineState::dwell(seconds);
  push(new DwellCommand(seconds));
}


void LinePlanner::move(const Axes &target, int axes, bool rapid) {
  Axes start = getPosition();

  LOG_DEBUG(3, "move(" << target << ", " << (rapid ? "true" : "false")
            << ") from " << start);

  // Check limits
  checkSoftLimits(target);

  MachineState::move(target, axes, rapid);

  double feed = rapid ? numeric_limits<double>::max() : getFeed();
  if (!feed) THROW("Non-rapid move with zero feed rate");

  // TODO Handle feed rate mode
  if (getFeedMode() != UNITS_PER_MINUTE)
    LOG_WARNING("Inverse time and units per rev feed modes are not supported");

  // Handle rapid auto off
  if (rapid && !rapidAutoOff && config.rapidAutoOff) {
    if (speed) pushSetCommand("speed", 0);
    rapidAutoOff = true;
  }

  if (!rapid && rapidAutoOff) {
    if (speed && !isnan(speed)) pushSetCommand("speed", speed);
    rapidAutoOff = false;
  }

  // Create line command
  LineCommand *lc =
    new LineCommand(start, target, feed, rapid, seeking, firstMove, config);

  // Update state
  seeking = false;
  firstMove = false;

  // Null or short move
  if (lc->length < config.minTravel) {
    if (lc->seeking) THROW("Seeking move too short");
    delete lc;
    return;
  }

  // Add it
  enqueue(lc, rapid);
}


void LinePlanner::pause(pause_t type) {
  MachineState::pause(type);
  push(new PauseCommand(type));
}


void LinePlanner::set(const string &name, double value, Units units) {
  double oldValue = MachineState::get(name, units);
  MachineState::set(name, value, units);

  if (name.length() == 2 && name[0] == '_' && Axes::isAxis(name[1])) return;
  if (oldValue == value && !String::endsWith(name, "_home")) return;
  if (name == "_speed") return;

  pushSetCommand(name, convert(units, METRIC, value));
}


void LinePlanner::setLocation(const LocationRange &location) {
  MachineState::setLocation(location);
  int line = location.getStart().getLine();

  if (0 <= line && line != this->line)
    pushSetCommand("line", this->line = line);
}


void LinePlanner::message(const string &s) {
  MachineState::message(s);
  pushSetCommand("message", s);
}


uint64_t LinePlanner::getNextID() {
  uint64_t id = nextID;
  nextID = (nextID + 1) & ((1U << config.idBits) - 1);
  return id;
}


bool LinePlanner::idLess(uint64_t a, uint64_t b) const {
  // Compare IDs with wrap around
  return (1U << (config.idBits - 1)) < ((a - b) & ((1U << config.idBits) - 1));
}


template <typename T>
void LinePlanner::pushSetCommand(const string &name, const T &_value) {
  SmartPointer<JSON::Value> value = JSON::Factory().create(_value);

  // Merge with previous set command if possible
  auto cmd = pre.empty() ? cmds.back() : pre.back();
  for (; cmd; cmd = cmd->prev) {
    SetCommand *sc = dynamic_cast<SetCommand *>(cmd);
    if (!sc) break;
    if (sc->getName() == name) {
      sc->setValue(value);
      return;
    }
  }

  push(new SetCommand(name, value));
}


void LinePlanner::push(PlannerCommand *cmd) {
  bool flush = true;

  // Check if this is a non-flushing SetCommand
  SetCommand *sc = dynamic_cast<SetCommand *>(cmd);
  if (sc) {
    const string &name = sc->getName();
    flush = name != "line" && name != "speed" && name != "_feed";
  }

  // Flush pre-plan queue
  while (flush && !pre.empty()) {
    PlannerCommand *cmd = pre.pop_front();
    cmd->setID(getNextID());
    cmds.push_back(cmd);
    plan(cmd);
  }

  // Flush command when in exact stop mode, exit velocity is zero or unmergable
  auto lc = dynamic_cast<LineCommand *>(cmd);
  if (config.pathMode != EXACT_STOP_MODE && cmd->getExitVelocity() &&
      (!lc || lc->canMerge()) && (lc || !flush) && (!sc || !pre.empty()))
    pre.push_back(cmd);

  else {
    cmd->setID(getNextID());
    cmds.push_back(cmd);
    plan(cmd);
  }
}


bool LinePlanner::merge(LineCommand *next, LineCommand *prev,
                        double lastSpeed) {
  if (!prev->merge(*next, config, lastSpeed)) return false;

  // Delete new line and intervening commands from pre-plan queue
  delete next;
  while (pre.back() != prev) delete pre.pop_back();

  // Check if newly merged move is too short
  if (prev->length < config.minTravel) {
    // Save last speed
    lastSpeed = prev->speeds.size() ? prev->speeds.back().speed :
      numeric_limits<double>::quiet_NaN();

    // Delete degenerate move
    delete pre.pop_back();

    // Restore last speed
    if (!std::isnan(lastSpeed)) pushSetCommand("speed", lastSpeed);
  }

  // Restore last feed
  if (getFeed() != prev->feed && !prev->rapid)
    pushSetCommand("_feed", getFeed());

  return true;
}


double LinePlanner::computeMaxAccel(const Vector3D &v) const {
  Vector3D unit = v.normalize();
  double maxAccel = numeric_limits<double>::max();

  for (unsigned axis = 0; axis < 3; axis++)
    if (config.maxAccel[axis] && isfinite(config.maxAccel[axis])) {
      double a = fabs(config.maxAccel[axis] / unit[axis]);
      if (a < maxAccel) maxAccel = a;

      // Compute implied max accel from max jerk and max velocity
      double j = fabs(config.maxJerk[axis] / unit[axis]);
      double v = fabs(config.maxVel[axis] / unit[axis]);
      a = sqrt(2 * v * j);

      if (a < maxAccel) maxAccel = a;
    }

  return std::min(config.junctionAccel, maxAccel);
}


double LinePlanner::computeJunctionVelocity(const Vector3D &v,
                                            double radius) const {
  return sqrt(computeMaxAccel(v) * radius);
}


unsigned LinePlanner::blendSegments(double arcError, double arcAngle,
                                    double radius) {
  // Compute segment angle from allowed error
  double segAngle = 2 * acos(1 - arcError / radius);

  // Segment angle cannot be greater than 2Pi/3 because we need at least 3
  // segments in a full circle
  segAngle = std::min(2 * M_PI / 3, segAngle);

  // Compute integer number of segments that meets the error bound
  unsigned segments = (unsigned)ceil(arcAngle / segAngle);

  // Enforce a minimum segment length
  double cordLength = arcAngle * radius;
  double segLength = cordLength / segments;
  if (segLength < 0.1) segments = (unsigned)floor(cordLength / 0.1);

  return segments;
}


void LinePlanner::blend(LineCommand *next, LineCommand *prev,
                        double lastSpeed, int lastLine) {
  if (!next->canBlend() || !prev->canBlend() || next->feed != prev->feed)
    return;

  // Get unit vectors
  Vector3D unitA = -prev->unit.slice<3>();
  Vector3D unitB = next->unit.slice<3>();

  // Don't blend nearly colinear segments
  double cosTheta = unitA.dot(unitB);
  if (cosTheta < -0.99) return;

  // Compute arc between segments given the allowed error
  double error = config.maxBlendError * 0.99;
  double intersectAngle = acos(cosTheta);
  double sinHalfAngle = sin(intersectAngle / 2);
  double cosHalfAngle = cos(intersectAngle / 2);
  double length = error * cosHalfAngle / (1 - sinHalfAngle);

  // Recompute error if arc is too large
  if (next->length < length * 2 || prev->length < length * 2) {
    length = min(next->length, prev->length) / 2;
    error = length * (1 - sinHalfAngle) / cosHalfAngle;
  }

  if (!std::isfinite(length) || length < config.minTravel) return;

  // Compute radius and angle of arc
  double radius = error * sinHalfAngle / (1 - sinHalfAngle);
  double arcAngle = M_PI - intersectAngle;

  // Arc error cannot be greater than arc radius
  const double arcError =
    std::min(std::min(config.maxBlendError * 0.01, radius), config.maxArcError);

  // Compute segments and segment angle
  unsigned segments = blendSegments(arcError, arcAngle, radius);

  if (!segments) return;

  // Delete intervening commands from pre-plan queue
  while (pre.back() != prev) delete pre.pop_back();

  // Get intersection point before making cut
  Vector3D p = next->start.getXYZ();

  // Adjust move endpoints, must be after getting intersection point above
  vector<LineCommand::Speed> speeds;
  prev->cut(length, speeds, 0, true);
  if (!std::isnan(lastSpeed))
    speeds.push_back(LineCommand::Speed(length, lastSpeed));
  next->cut(length, speeds, length, false);

  // Scale offsets
  for (unsigned i = 0; i < speeds.size(); i++)
    speeds[i].offset /= 2 * length;

  // Find arc center
  Vector3D q1 = p + unitA * length;
  Vector3D q2 = p + unitB * length;
  Vector3D v = unitA.cross(unitB).cross(unitA).normalize() * radius;
  Vector3D c1 = q1 + v;
  Vector3D c2 = q1 - v;
  Vector3D center;
  if ((q2 - c1).lengthSquared() < (q2 - c2).lengthSquared())
    center = c1;
  else center = c2;

  // Compute arc using SLERP
  Vector3D arcStart = q1 - center;
  Vector3D arcEnd   = q2 - center;
  Axes start = prev->target;
  Axes target = start;
  double sinAngle = sin(arcAngle);
  double lastFract = 0;
  unsigned speedIdx = 0;

  prev->targetJunctionVel =
    computeJunctionVelocity(center - target.getXYZ(), radius);

  double segAngle = arcAngle / segments;
  for (unsigned i = 0; i < segments; i++) {
    double a = (1 + i) * segAngle;
    Vector3D v;

    if (i == segments - 1) v = next->start.getXYZ(); // Exact end
    else v = arcStart * (sin(arcAngle - a) / sinAngle) +
           arcEnd * (sin(a) / sinAngle) + center;

    target.setXYZ(v);

    LineCommand *lc =
      new LineCommand(start, target, prev->feed, prev->rapid, false, false,
                      config);

    lc->targetJunctionVel = computeJunctionVelocity(center - v, radius);

    double fract = a / arcAngle;
    double segFract = fract - lastFract;

    // Insert intervening line set command in middle
    if (lastFract < 0.5 && 0.5 <= fract && 0 <= lastLine)
      pushSetCommand("line", lastLine);

    // Insert speeds at correct offsets
    for (unsigned i = speedIdx; i < speeds.size(); i++)
      if (speeds[i].offset <= fract) {
        LineCommand::Speed s(0, speeds[i].speed);
        s.offset = lc->length * (speeds[i].offset - lastFract) / segFract;
        lc->speeds.push_back(s);
        speedIdx = i + 1;
      }

    push(lc);
    start = target;
    lastFract = fract;
  }

  // Handle last speed, if any
  if (speedIdx < speeds.size()) {
    LineCommand::Speed s(0, speeds.back().speed);
    next->speeds.insert(next->speeds.begin(), s);
  }
}


void LinePlanner::enqueue(LineCommand *lc, bool rapid) {
  // Search for previous move in pre-plan queue and record last speed change
  double lastSpeed = numeric_limits<double>::quiet_NaN();
  int lastLine = -1;

  for (PlannerCommand *cmd = pre.back(); cmd; cmd = cmd->prev) {
    LineCommand *prev = dynamic_cast<LineCommand *>(cmd);

    if (prev) {
      if (merge(lc, prev, lastSpeed)) return; // Merged
      blend(lc, prev, lastSpeed, lastLine);
      break;
    }

    // Save last speed to synchronize with merged move
    SetCommand *sc = dynamic_cast<SetCommand *>(cmd);
    if (!sc) break;
    if (sc->getName() == "speed" && isnan(lastSpeed) &&
        (!rapid || !config.rapidAutoOff))
      lastSpeed = sc->getValue().getNumber();

    else if (sc->getName() == "line" && lastLine == -1)
      lastLine = sc->getValue().getS32();

    else if (sc->getName() != "_feed") break;
  }

  // Add the move
  push(lc);
}


bool LinePlanner::isFinal(PlannerCommand *cmd) const {
  if (cmd->isFinal()) return true;
  PlannerCommand *start = cmd;

  // Set commands are only final if they are followed by a non-set command
  if (dynamic_cast<SetCommand *>(cmd)) {
    for (PlannerCommand *c = cmd; c; c = c->next)
      if (!dynamic_cast<SetCommand *>(c)) return true;

    return false;
  }

  double velocity = cmd->getExitVelocity();
  if (velocity <= Math::nextUp(0)) return true;

  // Check if there is enough velocity change in the following blocks to
  // decelerate to zero if necessary.
  unsigned count = 0;
  while (cmd->next) {
    cmd = cmd->next;

    velocity -= cmd->getDeltaVelocity();
    if (velocity <= Math::nextUp(0)) return true;

    if (config.maxLookahead <= ++count) {
      // We reached our limit, try to plan speed up from zero back to start
      velocity = 0;
      count = 0;

      while (cmd != start) {
        velocity = speedUp(cmd, velocity);
        cmd = cmd->prev;
        count++;

        if (cmd->getExitVelocity() <= velocity) {
          LOG_DEBUG(3, "Hit lookahead limit but successfully planned reverse "
                    "speed up from zero to " << cmd->getExitVelocity()
                    << " in " << count << " moves");

          // Mark intervening commands final
          while (cmd != start) {
            cmd->setFinal();
            cmd = cmd->prev;
          }

          cmd->setFinal();
          return true;
        }
      }

      THROW("Planner exceeded max lookahead (" << config.maxLookahead << ")");
    }
  }

  return false;
}


void LinePlanner::plan(PlannerCommand *cmd) {
  if (planOne(cmd)) {
    LOG_DEBUG(3, "Backplanning from " << cmd->getID());

    while (true) {
      if (!cmd->prev) THROW("Cannot backplan, previous move unavailable");
      cmd = cmd->prev;
      if (!planOne(cmd)) break;
    }
  }
}


bool LinePlanner::planOne(PlannerCommand *cmd) {
  LOG_DEBUG(3, "Planning " << cmd->getID());
  LOG_DEBUG(4, "Planning " << cmd->toString());

  // Set entry velocity when at beginning
  bool backplan = false;
  if (!cmd->prev) cmd->setEntryVelocity(lastExitVel);

  else {
    // Make sure entry and exit velocities match
    PlannerCommand *last = cmd->prev;

    if (cmd->getEntryVelocity() < last->getExitVelocity()) {
      last->setExitVelocity(cmd->getEntryVelocity());
      backplan = true;
    }

    cmd->setEntryVelocity(last->getExitVelocity());
  }

  // Done with non-move commands
  if (!dynamic_cast<LineCommand *>(cmd)) return backplan;

  LineCommand &lc = *dynamic_cast<LineCommand *>(cmd);
  double Vi = lc.getEntryVelocity();
  double Vt = lc.getExitVelocity();

  // Apply junction velocity limit
  if (Vi && cmd->prev)
    for (PlannerCommand *last = cmd->prev; last; last = last->prev) {
      if (!dynamic_cast<LineCommand *>(last)) continue;
      const LineCommand &lastLC = *dynamic_cast<LineCommand *>(last);

      double jv;

      if (lastLC.targetJunctionVel) jv = lastLC.targetJunctionVel;
      else jv = computeJunctionVelocity(lc.unit, lastLC.unit,
                                        config.junctionDeviation,
                                        config.junctionAccel);
      if (jv < Vi) {
#if DEBUG
        double cosTheta = -lc.unit.dot(lastLC.unit);
        double angle = acos(cosTheta) / M_PI * 180;

        LOG_DEBUG(5, "junctionVelocity=" << jv
                  << " unitA=" << lc.unit
                  << " unitB=" << lastLC.unit
                  << " lenA=" << lc.length
                  << " lenB=" << lastLC.length
                  << " cosTheta=" << cosTheta
                  << " angle=" << angle
                  << " id=" << lc.getID());
#endif

        Vi = jv;
        cmd->setEntryVelocity(Vi);
        cmd->prev->setExitVelocity(Vi);
        backplan = true;
      }

      break;
    }

  // Always plan from lower velocity to higher
  bool swapped = false;
  if (Vt < Vi) {
    swap(Vi, Vt);
    swapped = true;
  }

  // Compute minimum length for velocity change
  double length = computeLength(Vi, Vt, lc.maxAccel, lc.maxJerk);

  // Check if velocity change fits
  if (lc.length < Math::nextDown(length)) {
    // Velocity change does not fit, compute a lower target velocity
    length = lc.length; // New target velocity will fit exactly
    Vt = peakVelocity(Vi, lc.maxAccel, lc.maxJerk, length);

    // Update velocities
    if (swapped) {
      // Backplaning  necessary
      backplan = true;

      if (!cmd->prev) THROW("Cannot backplan, previous move unavailable");

      LOG_DEBUG(3, "Backplan: entryVel=" << lc.entryVel
                << " prev.exitVel=" << cmd->prev->getExitVelocity()
                << " Vt=" << Vt);

      lc.entryVel = Vt;
      cmd->prev->setExitVelocity(Vt);

    } else {
      lc.exitVel = Vt;
      if (cmd->next) cmd->next->setEntryVelocity(Vt);
    }
  }

  // Zero times
  for (int i = 0; i < 7; i++) lc.times[i] = 0;

  // Plan curve segments
  if ((0.95 * lc.length <= length && length <= Math::nextUp(lc.length)) ||
      lc.maxVel * 0.95 < Vt) {
    // Exact or near fit or target velocity is close to max, compute simple
    // velocity transition.
    double lengthRemain = lc.length -
      planVelocityTransition(Vi, Vt, lc.maxAccel, lc.maxJerk, lc.times);

    if (lengthRemain < -config.minTravel)
      THROW("Velocity transition exceeds length by " << -lengthRemain
             << "mm required=" << lc.length << "mm computed=" << length
             << "mm Vt=" << Vt);

    // If there is appreciable length left, add a constant velocity segment
    if (config.minTravel < lengthRemain) lc.times[3] = lengthRemain / Vt;

    // Record change in velocity
    lc.deltaV = Vt - Vi;

  } else {
    // Velocity change fits and a higher peak velocity can be achieved.
    // Search for a peak velocity that fits well.
    double peakVel = lc.maxVel;
    double maxVel = peakVel;
    double minVel = Vt;
    int rounds = 0;

    while (true) {
      double headLen = computeLength(Vi, peakVel, lc.maxAccel, lc.maxJerk);
      double tailLen = computeLength(Vt, peakVel, lc.maxAccel, lc.maxJerk);
      double bodyLen = lc.length - headLen - tailLen;

      if (0 <= bodyLen) {
        // Fits, stop if we are close enough or there have been enough rounds
        if (0.99 * maxVel < peakVel || 16 < rounds) break;
        rounds++;

        // Try a higher velocity
        minVel = peakVel;
        peakVel = peakVel + (maxVel - peakVel) / 2;

      } else {
        // Does not fit, try a lower velocity
        maxVel = peakVel;
        peakVel = minVel + (peakVel - minVel) / 2;

        // Quit trying if we are really close to minVel
        if (peakVel < minVel + 0.0001) {
          LOG_DEBUG(3, "peakVel=" << peakVel << " bodyLen=" << bodyLen
                    << " length=" << length << " lc.length=" << lc.length);
          peakVel = minVel;
          break;
        }
      }
    }

    // Plan s-curve
    double length = lc.length;
    length -= planVelocityTransition(Vi, peakVel, lc.maxAccel, lc.maxJerk,
                                     lc.times);
    length -= planVelocityTransition(peakVel, Vt, lc.maxAccel, lc.maxJerk,
                                     lc.times + 4);
    if (config.minTravel < length) lc.times[3] = length / peakVel;

    // Record change in velocity
    lc.deltaV = peakVel - Vi + peakVel - Vt;
  }

  // Reverse the plan, if velocities were swapped above
  if (swapped) {
    for (int i = 0; i < 3; i++)
      swap(lc.times[i], lc.times[6 - i]);

    swap(Vi, Vt);
  }

  // Use what we could accelerate to as an upper bound
  double deltaV = peakVelocity(Vt, lc.maxAccel, lc.maxJerk, lc.length) - Vt;
  if (lc.deltaV < deltaV) lc.deltaV = deltaV;

  return backplan;
}


bool LinePlanner::isAccelLimited(double Vi, double Vt, double maxAccel,
                                 double maxJerk) const {
  return Vi + square(maxAccel) / maxJerk < Math::nextDown(Vt);
}


double LinePlanner::peakAccelFromDeltaV(double Vi, double Vt,
                                        double jerk) const {
  double Ap = sqrt(fabs(jerk * (Vt - Vi)));

  LOG_DEBUG(3, "peakAccelFromDeltaV(" << Vi << ", " << Vt << ", " << jerk
            << ") = " << Ap);

  return Ap;
}


// Peak ramp up ramp down acceleration over provided length
double LinePlanner::peakAccelFromLength(double Vi, double jerk,
                                        double length) const {
  // Start with the formula for travel distance (length):
  //
  //   L = Vi * t + Jm * t^3 / 6 + Vh * t + Ap * t^2 / 2 - Jm * t^3 / 6
  //   L = Vi * t + Vh * t + Ap * t^2 / 2
  //
  // Where:
  //
  //   t  = The ramp up, ramp down time.  Total time T = t * 2.
  //   L  = length
  //   Jm = jerk
  //   Ap = The peak acceleration we are looking for.
  //   Vh = The velocity at peak acceleration.
  //
  // Then the formula for Vh is:
  //
  //   Vh = Vi + Jm * t^2 / 2
  //
  // Substitute Vh:
  //
  //   L = Vi * t + (Vi + Jm * t^2 / 2) * t + Ap * t^2 / 2
  //   L = 2 * Vi * t + Jm * t^3 / 2 + Ap * t^2 / 2
  //
  // The time t is just:
  //
  //   t = Ap / Jm
  //
  // Substitute t:
  //
  //   1 / Jm^2 * Ap^3 + 2 * Vi / Jm * Ap - L = 0
  //
  // Solve for Ap by the cubic formula:
  //
  //   x = (q + (q^2 + (r - p^2)^3)^1/2)^1/3 +
  //       (q - (q^2 + (r - p^2)^3)^1/2)^1/3 + p
  //
  // Where:
  //
  //   p = -b / (3 * a)
  //   q = p^3 + (b * c - 3 * a * d) / (6 * a^2)
  //   r = c / (3 *a)
  //
  // The standard cubic polynomal looks like this:
  //
  //   a * x^3 + b * x^2 + c * x + d = 0
  //
  // So our coefficients are:
  //
  //   a = 1 / Jm^2
  //   b = 0
  //   c = 2 * Vi / Jm
  //   d = -L
  //
  // And:
  //
  //   x = Ap
  //
  // So we have:
  //
  //   p = 0
  //   q = 1/2 * L * Jm^2
  //   r = 2/3 * Vi * Jm
  //
  //--------------------------------------------------------------------------
  // Since negative velocity is impossible, we never want a decceleration
  // that would lead to a negative ending velocity.  When the target velocity
  // Vt is zero we have:
  //
  //   Vt = Vi + Ap^2 / Jm = 0
  //   Jm = -Ap^2 / Vi
  //
  // If we plug this into our original formula for Ap we get:
  //
  //   1 / (-Ap^2 / Vi)^2 * Ap^3 + 2 * Vi / (-Ap^2 / Vi) * a - L = 0
  //
  // Which simplifies to:
  //
  //   Ap = -Vi^2 / L
  //
  // So our target velocity is zero when:
  //
  //   Jm = -Vi^3 / L^2
  //
  // Note that when Jm is negative the following can be negative:
  //
  //   q^2 + (r - p^2)^3
  //   (1/2 * L * Jm^2)^2 + (2/3 * Vi * Jm)^3
  //
  // In fact, it is always negative when Jm < 0 and 0 < Vt.  Proof omitted.
  // So we must handle complex numbers correctly.

  if (jerk < 0 && jerk <= -cube(Vi) / square(length))
    return -square(Vi) / length; // Peak accel when Vt = 0

  complex<double> q = 0.5 * length * square(jerk);
  complex<double> r = 2.0 / 3.0 * Vi * jerk;
  complex<double> sqrtq2r3 = sqrt(square(q) + cube(r));

  complex<double> Ap = cbrt(q + sqrtq2r3) + cbrt(q - sqrtq2r3);

  LOG_DEBUG(3, "peakAccelFromLength(" << Vi << ", " << jerk << ", "
            << length << ") = " << Ap);

  if (!isfinite(Ap.real()) || Ap.imag())
    THROW("Invalid peak acceleration length=" << length);

  return Ap.real();
}


double LinePlanner::peakVelocity(double Vi, double maxAccel, double maxJerk,
                                 double length) const {
  double peakAccel = peakAccelFromLength(Vi, maxJerk, length);
  double Vp;

  if (fabs(maxAccel) < Math::nextDown(fabs(peakAccel))) {
    // With constant accel period
    //
    // Solve:
    //
    //   Jm * Vp^2 + As^2 * Vp + As^2 * Vi - Vi^2 * Jm - 2As * Jm * L = 0
    //
    // for Vp using the quadradic formula
    double a = maxJerk;
    double b = square(maxAccel);
    double c =
      b * Vi - square(Vi) * maxJerk - 2 * maxAccel * maxJerk * length;
    Vp = (-b + sqrt(square(b) - 4 * a * c)) / (2 * a);

  } else Vp = Vi + square(peakAccel) / maxJerk; // No constant accel period

  if (!isfinite(Vp)) THROW("Invalid peak velocity");
  if (Vp <= Math::nextUp(0)) Vp = 0;

  LOG_DEBUG(3, "peakVelocity(" << Vi << ", " << maxAccel << ", " << maxJerk
            << ", length=" << length << ") = " << Vp << " with"
            << (maxAccel < peakAccel ? "" : " out") << " constant accel");

  return Vp;
}


double LinePlanner::speedUp(PlannerCommand *cmd, double Vi) const {
  auto *lc = dynamic_cast<LineCommand *>(cmd);
  if (!lc) return Vi;

  double length = lc->length;
  double jerk = lc->maxJerk;
  double maxAccel = lc->maxAccel;
  double peakAccel = peakAccelFromLength(Vi, jerk, length);

  if (fabs(maxAccel) < Math::nextDown(fabs(peakAccel))) {
    // With constant accel period
    double t = maxAccel / jerk;
    double v = Vi + SCurve::velocity(t, 0, jerk);
    length -= SCurve::distance(t, Vi, 0, jerk);

    // Find time of constant accel period (T) by solving:
    //
    //  v * T + 1/2 * a * T^2 + vh * T + 1/2 * a * t^2 - 1/6 * j * t^3 = L
    //
    // Where:
    //
    //  a  = maxAccel
    //  j  = jerk;
    //  vh = t * a
    //
    // This reduces to the quadratic equation:
    //
    //  A * T^2 + B * T + C = 0

    double A = 0.5 * maxAccel;
    double B = v + maxAccel * t;
    double C = 2.0 / 3.0 * maxAccel * t * t - length;
    double T = (-B + sqrt(B * B - 4 * A * C)) / (2 * A);

    return v + SCurve::velocity(T, maxAccel, 0);
  }

  double t = peakAccel / jerk;
  return
    Vi + SCurve::velocity(t, 0, jerk) + SCurve::velocity(t, peakAccel, -jerk);
}


double LinePlanner::computeLength(double Vi, double Vt, double maxAccel,
                                  double maxJerk) const {
  double length;

  // Compute length for velocity change
  if (isAccelLimited(Vi, Vt, maxAccel, maxJerk))
    // With constant acceleration segment
    length = (Vi + Vt) * (square(maxAccel) + maxJerk * (Vt - Vi)) /
      (2 * maxAccel * maxJerk);

  else // With out constant acceleration
    length = sqrt(Vt - Vi) * (Vi + Vt) / sqrt(maxJerk);

  LOG_DEBUG(3, "computeLength(" << Vi << ", " << Vt << ", " << maxAccel
            << ", " << maxJerk << ") = " << length);

  if (!isfinite(length)) THROW("Invalid length from velocity change");

  return length;
}


double LinePlanner::planVelocityTransition(double Vi, double Vt,
                                           double maxAccel, double maxJerk,
                                           double *times) const {
  // Compute from lowest to highest velocity
  if (Vt < Vi) swap(Vi, Vt);

  // Compute maximum acceleration
  double peakAccel = peakAccelFromDeltaV(Vi, Vt, maxJerk);
  if (maxAccel < peakAccel) peakAccel = maxAccel;

  // Acceleration segment
  times[0] = peakAccel / maxJerk;
  double length = SCurve::distance(times[0], Vi, 0, maxJerk);
  double vel = Vi + SCurve::velocity(times[0], 0, maxJerk);

  // Constant acceleration segment
  if (isAccelLimited(Vi, Vt, peakAccel, maxJerk)) {
    times[1] = (Vt - Vi) / peakAccel - times[0];
    if (times[1] < 1e-12) times[1] = 0; // Handle floating-point rounding errors
    length += SCurve::distance(times[1], vel, peakAccel, 0);
    vel += SCurve::velocity(times[1], peakAccel, 0);

  } else times[1] = 0;

  // Decceleration segment
  times[2] = times[0];
  length += SCurve::distance(times[0], vel, peakAccel, -maxJerk);

  LOG_DEBUG(3, "planVelocityTransition(" << Vi << ", " << Vt << ", "
            << maxAccel << ", " << maxJerk << ")=" << length);

  return length;
}


double LinePlanner::computeJunctionVelocity
(const Axes &unitA, const Axes &unitB, double deviation, double accel) const {
  // TODO this does not make sense for axes A, B, C, U, V or W
  double cosTheta = -unitA.dot(unitB);

  if (cosTheta < -0.99) return numeric_limits<double>::max(); // Straight line
  if (0.99 < cosTheta) return 0; // Reversal

  double theta = acos(cosTheta);
  double sinHalfTheta = sin(theta / 2);
  double radius = deviation * sinHalfTheta / (1 - sinHalfTheta);

  return sqrt(radius * accel);
}
