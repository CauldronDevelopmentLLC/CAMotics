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
#include "SpeedCommand.h"
#include "ToolCommand.h"
#include "DwellCommand.h"
#include "PauseCommand.h"
#include "LineNumberCommand.h"

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/json/Sink.h>

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


bool LinePlanner::hasMove() const {
  return !cmds.empty() && isFinal(cmds.begin());
}


void LinePlanner::next(JSON::Sink &sink) {
  if (!hasMove()) THROW("Planner not ready");

  SmartPointer<PlannerCommand> cmd = cmds.front();
  cmd->write(sink);
  output.push_back(cmd);
  cmds.pop_front();
}


void LinePlanner::release(uint64_t id) {
  while (!output.empty() && output.front()->getID() <= id)
    output.pop_front();
}


void LinePlanner::restart(uint64_t id, double length) {
  // Find replan command in output
  while (true) {
    if (output.empty() || id < output.front()->getID())
      THROWS("Planner ID " << id << " at length " << length
             << " not found");

    if (output.front()->getID() == id) {
      if (output.front()->getLength() <= length) break;
      else length -= output.front()->getLength();
    }

    output.pop_front(); // Release any moves before the restart
  }

  // Reload previously output moves
  cmds.splice(cmds.begin(), output, output.begin(), output.end());

  // Reset output position
  outputPos = Vector4D(numeric_limits<double>::quiet_NaN());

  // Replan from zero velocity
  cmds.front()->restart(length);
  for (auto it = cmds.begin(); it != cmds.end(); it++)
    plan(it);
}


void LinePlanner::start() {
  for (int i = 0; i < 4; i++)
    position[i] = outputPos[i] = config.start[i];

  lastExitVel = 0;
  lastUnit = Vector4D();

  MachineAdapter::start();
}


void LinePlanner::end() {
  MachineAdapter::end();

  if (!cmds.empty()) {
    auto it = std::prev(cmds.end());
    (*it)->setExitVelocity(0);
    plan(it);
  }
}


void LinePlanner::setSpeed(double speed, spin_mode_t mode, double max) {
  MachineAdapter::setSpeed(speed, mode, max);
  // TODO handle spin mode
  push(new SpeedCommand(nextID++, speed));
}


void LinePlanner::setTool(unsigned tool) {
  MachineAdapter::setTool(tool);
  push(new ToolCommand(nextID++, tool));
}


void LinePlanner::dwell(double seconds) {
  MachineAdapter::dwell(seconds);
  push(new DwellCommand(nextID++, seconds));
}


void LinePlanner::move(const Axes &target, bool rapid) {
  MachineAdapter::move(target, rapid);

  SmartPointer<LineCommand> lc = new LineCommand(nextID++);

  for (int i = 0; i < 4; i++)
    lc->position[i] = target[i];

  // Compute axis and lengths
  Vector4D delta = lc->position - position;
  position = lc->position;
  lc->length = delta.length();

  // TODO ignore too short moves
  if (!lc->length) return; // Null move
  if (!isfinite(lc->length)) THROWS("Invalid length");

  Vector4D unit = delta / lc->length;

  // Apply user velocity limit
  // TODO Handle feed rate mode
  if (!rapid) {
    lc->maxVel = getFeed();
    if (!lc->maxVel) THROWS("Non-rapid move with zero feed rate");
  }

  // Apply axis velocity limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && isfinite(config.maxVel[i])) {
      double v = fabs(config.maxVel[i] / unit[i]);
      if (v < lc->maxVel) lc->maxVel = v;
    }

  // Apply axis jerk limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && isfinite(config.maxJerk[i])) {
      double j = fabs(config.maxJerk[i] / unit[i]);
      if (j < lc->maxJerk) lc->maxJerk = j;
    }

  // Apply axis acceleration limits
  for (unsigned i = 0; i < 4; i++)
    if (unit[i] && isfinite(config.maxAccel[i])) {
      double a = fabs(config.maxAccel[i] / unit[i]);
      if (a < lc->maxAccel) lc->maxAccel = a;
    }

  // Apply junction velocity limit
  if (lastUnit != Vector4D()) {
    double jv = computeJunctionVelocity(unit, lastUnit,
                                        config.junctionDeviation,
                                        config.junctionAccel);
    if (jv < lc->entryVel) lc->entryVel = jv;
  }
  lastUnit = unit;

  // Limit velocity
  if (lc->maxVel < lc->entryVel) lc->entryVel = lc->maxVel;
  if (lc->maxVel < lc->exitVel) lc->exitVel = lc->maxVel;

  // Add move
  push(lc);
}


void LinePlanner::pause(bool optional) {
  MachineAdapter::pause(optional);
  push(new PauseCommand(nextID++, optional));
}


void LinePlanner::setLocation(const LocationRange &location) {
  MachineAdapter::setLocation(location);
  int line = location.getStart().getLine();

  if (0 <= line && line != this->line) {
    this->line = line;
    push(new LineNumberCommand(nextID++, line));
  }
}


void LinePlanner::push(const cb::SmartPointer<PlannerCommand> &cmd) {
  cmds.push_back(cmd);

  // Plan move
  if (lastExitVel < cmd->getEntryVelocity()) cmd->setEntryVelocity(lastExitVel);
  plan(std::prev(cmds.end()));

  lastExitVel = cmd->getExitVelocity();
}


bool LinePlanner::isFinal(cmds_t::const_iterator it) const {
  double velocity = (*it)->getExitVelocity();
  if (!velocity) return true;

  // Check if there is enough velocity change in the following blocks to
  // deccelerate to zero if necessary.
  while (++it != cmds.end()) {
    velocity -= (*it)->getDeltaVelocity();
    if (velocity <= 0) return true;
  }

  return false;
}


void LinePlanner::plan(cmds_t::iterator it) {
  if (planOne(it))
    // Backplan
    while (true) {
      if (it == cmds.begin())
        THROWS("Cannot backplan, previous move unavailable");

      if (!planOne(--it)) break;
    }
}


bool LinePlanner::planOne(cmds_t::iterator it) {
  const SmartPointer<PlannerCommand> &pc = *it;

  // Plan non-move commands
  if (!pc.isInstance<LineCommand>()) {
    if (it != cmds.begin() &&
        pc->getEntryVelocity() < (*std::prev(it))->getExitVelocity()) {
      (*std::prev(it))->setExitVelocity(pc->getEntryVelocity());
      return true;
    }

    return false;
  }

  LineCommand &lc = *it->cast<LineCommand>();
  bool backplan = false;
  double Vi = lc.entryVel;
  double Vt = lc.exitVel;

  // Check that entry velocity is not less than last exit velocity
  if (it != cmds.begin() && Vi < (*std::prev(it))->getExitVelocity()) {
    (*std::prev(it))->setExitVelocity(Vi);
    backplan = true;
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

      if (it == cmds.begin())
        THROWS("Cannot backplan, previous move unavailable");

      LOG_DEBUG(3, "Backplan: entryVel=" << lc.entryVel
                << " prev.exitVel=" << (*std::prev(it))->getExitVelocity()
                << " Vt=" << Vt);

      lc.entryVel = Vt;
      (*std::prev(it))->setExitVelocity(Vt);

    } else {
      lc.exitVel = Vt;
      if (std::next(it) != cmds.end()) (*std::next(it))->setEntryVelocity(Vt);
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
  if (swapped)
    for (int i = 0; i < 3; i++)
      swap(lc.times[i], lc.times[6 - i]);

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

  if (maxAccel < peakAccel) {
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


double LinePlanner::computeJunctionVelocity(const Vector4D &unitA,
                                            const Vector4D &unitB,
                                            double deviation,
                                            double accel) const {
  double cosTheta = -unitA.dot(unitB);

  if (cosTheta < -0.99) return numeric_limits<double>::max(); // Straight line
  if (0.99 < cosTheta) return 0; // Reversal

  // Fuse the junction deviations into a vector sum
  double aDelta = 0;
  double bDelta = 0;

  for (int axis = 0; axis < 4; axis++) {
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
