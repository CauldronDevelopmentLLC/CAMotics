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

#include "LinePlanner.h"

#include "LineCommand.h"
#include "DwellCommand.h"
#include "PauseCommand.h"
#include "OutputCommand.h"
#include "SeekCommand.h"
#include "SetCommand.h"

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
  template<typename T> T square(T x) {return x * x;}
  template<typename T> T cube(T x) {return x * x * x;}


  complex<double> cbrt(complex<double> x) {
    return x.real() < 0 ? -pow(-x, 1.0 / 3.0) : pow(x, 1.0 / 3.0);
  }


  double computeDistance(double t, double v, double a, double j) {
    // v * t + 1/2 * a * t^2 + 1/6 * j * t^3
    return t * (v + t * (0.5 * a + 1.0 / 6.0 * j * t));
  }


  double computeVelocity(double t, double a, double j) {
    // a * t + 1/2 * j * t^2
    return t * (a + 0.5 * j * t);
  }
}


LinePlanner::LinePlanner() :
  lastExitVel(0), seeking(false), nextID(1), line(-1) {}


void LinePlanner::setConfig(const PlannerConfig &config) {
  this->config = config;
}


bool LinePlanner::isDone() const {return cmds.empty();}


bool LinePlanner::hasMove() const {
  return !cmds.empty() && isFinal(cmds.front());
}


uint64_t LinePlanner::next(JSON::Sink &sink) {
  if (!hasMove()) THROW("Planner not ready");

  PlannerCommand *cmd = cmds.front();
  cmd->write(sink);
  out.push_back(cmds.pop_front());
  lastExitVel = cmd->getExitVelocity();

  return cmd->getID();
}


void LinePlanner::setActive(uint64_t id) {
  while (!out.empty() && out.front()->getID() < id)
    delete out.pop_front();
}


bool LinePlanner::restart(uint64_t id, const Axes &position) {
  // Find replan command in output
  while (true) {
    if (out.empty() || id < out.front()->getID())
      THROWS("Planner ID " << id << " not found");

    if (out.front()->getID() == id) break;

    delete out.pop_front(); // Release any moves before the restart
  }

  // Reload previously output moves
  while (!out.empty()) cmds.push_front(out.pop_back());

  // Reset last exit velocity
  lastExitVel = 0;

  // Handle restart after seek
  PlannerCommand *cmd = cmds.front();
  if (cmd->isSeeking()) {
    // Skip reset of current move
    cmd = cmd->next;
    delete cmds.pop_front();
    // Replan next move, if one has already been planned.  Its start position
    // may have changed.
    while (cmd && !cmd->isMove()) cmd = cmd->next;
  }

  if (!cmd) return false; // Nothing to replan

  // Replan from zero velocity
  cmd->restart(position, config);
  for (cmd = cmds.front(); cmd->next; cmd = cmd->next)
    plan(cmd);

  return true;
}


void LinePlanner::start() {
  lastExitVel = 0;
  MachineState::start();
}


void LinePlanner::end() {
  MachineState::end();

  if (!cmds.empty()) {
    cmds.back()->setExitVelocity(0);
    plan(cmds.back());
  }
}


void LinePlanner::setSpeed(double speed) {
  MachineState::setSpeed(speed);

  // TODO handle spin mode
  if (getSpinMode() != REVOLUTIONS_PER_MINUTE)
    LOG_WARNING("Constant surface speed not supported");

  pushSetCommand("speed", speed);
}


void LinePlanner::changeTool(unsigned tool) {pushSetCommand("tool", tool);}


void LinePlanner::seek(port_t port, bool active, bool error) {
  MachineState::seek(port, active, error);
  push(new SeekCommand(nextID++, port, active, error));
  seeking = true;
}



void LinePlanner::output(port_t port, double value) {
  MachineState::output(port, value);
  push(new OutputCommand(nextID++, port, value));
}


void LinePlanner::dwell(double seconds) {
  MachineState::dwell(seconds);
  push(new DwellCommand(nextID++, seconds));
}


void LinePlanner::move(const Axes &target, bool rapid) {
  Axes start = getPosition();

  LOG_DEBUG(3, "move(" << target << ", " << (rapid ? "true" : "false")
            << ") from " << start);

  MachineState::move(target, rapid);

  // TODO Handle feed rate mode
  double feed = rapid ? numeric_limits<double>::max() : getFeed();
  if (!feed) THROWS("Non-rapid move with zero feed rate");

  if (getFeedMode() != UNITS_PER_MINUTE)
    LOG_WARNING("Inverse time and units per rev feed modes are not supported");

  LineCommand *lc =
    new LineCommand(nextID++, start, target, feed, seeking, config);

  // Update state
  seeking = false;

  // Null move
  if (!lc->length) {
    delete lc;
    return;
  }

  // Add move
  push(lc);
}


void LinePlanner::pause(bool optional) {
  MachineState::pause(optional);
  push(new PauseCommand(nextID++, optional));
}


void LinePlanner::set(const string &name, double value) {
  if (MachineState::get(name) == value) return;
  MachineState::set(name, value);
  pushSetCommand(name, value);
}


void LinePlanner::setLocation(const LocationRange &location) {
  MachineState::setLocation(location);
  int line = location.getStart().getLine();

  if (0 <= line && line != this->line)
    pushSetCommand("line", this->line = line);
}


template <typename T>
void LinePlanner::pushSetCommand(const string &name, const T &value) {
  push(new SetCommand(nextID++, name, JSON::Factory::create(value)));
}


void LinePlanner::push(PlannerCommand *cmd) {
  cmds.push_back(cmd);
  plan(cmd);
}


bool LinePlanner::isFinal(PlannerCommand *cmd) const {
  double velocity = cmd->getExitVelocity();
  if (!velocity) return true;

  // Check if there is enough velocity change in the following blocks to
  // deccelerate to zero if necessary.
  unsigned count = 0;
  while (cmd->next) {
    cmd = cmd->next;
    velocity -= cmd->getDeltaVelocity();
    if (velocity <= 0) return true;
    if (config.maxLookahead <= ++count)
      THROWS("Planner exceeded max lookahead (" << config.maxLookahead << ")");
  }

  return false;
}


void LinePlanner::plan(PlannerCommand *cmd) {
  if (planOne(cmd))
    // Backplan
    while (true) {
      if (!cmd->prev) THROWS("Cannot backplan, previous move unavailable");
      cmd = cmd->prev;
      if (!planOne(cmd)) break;
    }
}


bool LinePlanner::planOne(PlannerCommand *cmd) {
  LOG_DEBUG(4, "Planning " << cmd->toString());

  // Set entry velocity when at begining
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
  if (Vi && cmd->prev) {
    PlannerCommand *last = cmd->prev;
    while (true) {
      if (dynamic_cast<LineCommand *>(last)) {
        const Axes &lastUnit = dynamic_cast<LineCommand *>(last)->unit;

        double jv = computeJunctionVelocity(lc.unit, lastUnit,
                                            config.junctionDeviation,
                                            config.junctionAccel);
        if (jv < Vi) {
          Vi = jv;
          cmd->setEntryVelocity(Vi);
          cmd->prev->setExitVelocity(Vi);
          backplan = true;
        }
        break;
      }

      if (!last->prev) break;
      last = last->prev;
    }
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
  if (lc.length < length) {
    // Velocity change does not fit, compute a lower target velocity
    length = lc.length; // New target velocity will fit exactly
    Vt = peakVelocity(Vi, lc.maxAccel, lc.maxJerk, length);

    // Update velocities
    if (swapped) {
      // Backplaning  necessary
      backplan = true;

      if (!cmd->prev)
        THROWS("Cannot backplan, previous move unavailable");

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
  if ((0.95 * lc.length <= length && length <= lc.length) ||
      lc.maxVel * 0.95 < Vt) {
    // Exact or near fit or target velocity is close to max, compute simple
    // velocity transition.
    double lengthRemain = lc.length -
      planVelocityTransition(Vi, Vt, lc.maxAccel, lc.maxJerk, lc.times);

    if (lengthRemain < -0.000001)
      THROWS("Velocity transition exceeds length by " << -lengthRemain
             << " required=" << lc.length << " computed=" << length
             << " Vt=" << Vt);

    // If there is length left, add a constant velocity segment
    if (0.000001 < lengthRemain) lc.times[3] = lengthRemain / Vt;

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
      }
    }

    // Plan s-curve
    double length = lc.length;
    length -= planVelocityTransition(Vi, peakVel, lc.maxAccel, lc.maxJerk,
                                     lc.times);
    length -= planVelocityTransition(peakVel, Vt, lc.maxAccel, lc.maxJerk,
                                     lc.times + 4);
    lc.times[3] = length / peakVel;

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
  return Vi + square(maxAccel) / maxJerk < Vt;
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

  if (!isfinite(Ap.real()) || Ap.imag()) THROW("Invalid peak acceleration");

  return Ap.real();
}


double LinePlanner::peakVelocity(double Vi, double maxAccel, double maxJerk,
                                 double length) const {
  double peakAccel = peakAccelFromLength(Vi, maxJerk, length);
  double Vp;

  if (fabs(maxAccel) < fabs(peakAccel)) {
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
  if (Vp < 0) Vp = 0;

  LOG_DEBUG(3, "peakVelocity(" << Vi << ", " << maxAccel << ", " << maxJerk
            << ", length=" << length << ") = " << Vp << " with"
            << (maxAccel < peakAccel ? "" : " out") << " constant accel");

  return Vp;
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
  double length = computeDistance(times[0], Vi, 0, maxJerk);
  double vel = Vi + computeVelocity(times[0], 0, maxJerk);

  // Constant acceleration segment
  if (isAccelLimited(Vi, Vt, peakAccel, maxJerk)) {
    times[1] = (Vt - Vi) / peakAccel - times[0];
    length += computeDistance(times[1], vel, peakAccel, 0);
    vel += computeVelocity(times[1], peakAccel, 0);

  } else times[1] = 0;

  // Decceleration segment
  times[2] = times[0];
  length += computeDistance(times[0], vel, peakAccel, -maxJerk);

  LOG_DEBUG(3, "planVelocityTransition(" << Vi << ", " << Vt << ", "
            << maxAccel << ", " << maxJerk << ")=" << length);

  return length;
}


double LinePlanner::computeJunctionVelocity(const Axes &unitA,
                                            const Axes &unitB,
                                            double deviation,
                                            double accel) const {
  // TODO this probably does not make sense for axes A, B, C, U, V or W
  double cosTheta = -unitA.dot(unitB);

  if (cosTheta < -0.99) return numeric_limits<double>::max(); // Straight line
  if (0.99 < cosTheta) return 0; // Reversal

  // Fuse the junction deviations into a vector sum
  double aDelta = 0;
  double bDelta = 0;

  for (unsigned axis = 0; axis < Axes::getSize(); axis++) {
    aDelta += square(unitA[axis] * deviation);
    bDelta += square(unitB[axis] * deviation);
  }

  LOG_DEBUG(3, "delta A=" << aDelta << " delta B=" << bDelta);

  if (!aDelta || !bDelta) return 0; // A unit vector is null

  double delta = (sqrt(aDelta) + sqrt(bDelta)) / 2;
  double theta2 = acos(cosTheta) / 2;
  double radius = delta * sin(theta2) / (1 - sin(theta2));

  return sqrt(radius * accel);
}
