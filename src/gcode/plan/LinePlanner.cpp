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

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>

#include <limits>
#include <algorithm>
#include <complex>

using namespace cb;
using namespace std;
using namespace GCode;


namespace {
  template<typename T> T square(T x) {return x * x;}
  template<typename T> T cube(T x) {return x * x * x;}


  double computeDistance(double t, double v, double a, double j) {
    // v * t + 1/2 * a * t^2 + 1/6 * j * t^3
    return t * (v + t * (0.5 * a + 1.0 / 6.0 * j * t));
  }


  double computeVelocity(double t, double a, double j) {
    // a * t + 1/2 * j * t^2
    return t * (a + 0.5 * j * t);
  }


  bool isAccelLimited(double Vi, double Vt, double maxAccel, double maxJerk) {
    return Vi + square(maxAccel) / maxJerk < Vt;
  }


  double peakAccelFromDeltaV(double Vi, double Vt, double maxJerk) {
    double Ap = sqrt(fabs(maxJerk * (Vt - Vi)));

    LOG_DEBUG(3, "peakAccelFromDeltaV(" << Vi << ", " << Vt << ", " << maxJerk
              << ") = " << Ap);

    return Ap;
  }


  // Peak ramp up ramp down acceleration over provided length
  double peakAccelFromLength(double Vi, double maxJerk, double length) {
    double q = 0.5 * length * square(maxJerk);
    double r = 2.0 / 3.0 * Vi * maxJerk;
    double sqrtq2r3 = sqrt(square(q) + cube(r));

    double Ap = cbrt(q + sqrtq2r3) + cbrt(q - sqrtq2r3);

    LOG_DEBUG(3, "peakAccelFromLength(" << Vi << ", " << maxJerk << ", "
              << length << ") = " << Ap);

    if (!isfinite(Ap)) THROW("Invalid peak acceleration");

    return Ap;
  }


  double peakVelocity(double Vi, double maxAccel, double maxJerk,
                      double length) {
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


  double computeLength(double Vi, double Vt, double maxAccel, double maxJerk) {
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


  double planVelocityTransition(double Vi, double Vt, double maxAccel,
                                double maxJerk, double *times) {
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


  double computeJunctionVelocity(const Vector4D &unitA, const Vector4D &unitB,
                                 double deviation, double accel) {
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
}


LinePlanner::Point::Point() :
  length(0), entryVel(numeric_limits<double>::max()),
  exitVel(numeric_limits<double>::max()), deltaV(0),
  maxVel(numeric_limits<double>::max()),
  maxAccel(numeric_limits<double>::max()),
  maxJerk(numeric_limits<double>::max()) {}


void LinePlanner::start() {
  for (int i = 0; i < 4; i++)
    position[i] = execPos[i] = config.start[i];

  lastExitVel = 0;
  lastUnit = Vector4D();

  MachineAdapter::start();
  sink.start();
}


void LinePlanner::end() {
  MachineAdapter::end();

  if (!points.empty()) {
    points.back().exitVel = 0;
    plan(prev(points.end()));
    exec();
  }

  sink.end();
}


void LinePlanner::move(const Axes &target, bool rapid) {
  MachineAdapter::move(target, rapid);

  Point p;

  for (int i = 0; i < 4; i++)
    p.position[i] = target[i];

  // Compute axis and point lengths
  Vector4D delta = p.position - position;
  position = p.position;
  p.length = delta.length();
  p.unit = delta / p.length;

  if (!p.length) return; // Null move

  // Apply user velocity limit
  // TODO Handle feed rate mode
  if (!rapid) {
    p.maxVel = getFeed();
    if (!p.maxVel) THROWS("Non-rapid move with zero feed rate");
  }

  // Apply axis velocity limits
  for (unsigned i = 0; i < 4; i++)
    if (p.unit[i]) {
      double v = fabs(config.maxVel[i] / p.unit[i]);
      if (v < p.maxVel) p.maxVel = v;
    }

  // Apply axis acceleration limits
  for (unsigned i = 0; i < 4; i++)
    if (p.unit[i]) {
      double a = fabs(config.maxAccel[i] / p.unit[i]);
      if (a < p.maxAccel) p.maxAccel = a;
    }

  // Apply axis jerk limits
  for (unsigned i = 0; i < 4; i++)
    if (p.unit[i]) {
      double j = fabs(config.maxJerk[i] / p.unit[i]);
      if (j < p.maxJerk) p.maxJerk = j;
    }

  // Apply junction velocity limit
  if (lastUnit != Vector4D()) {
    double jv = computeJunctionVelocity(p.unit, lastUnit,
                                        config.junctionDeviation,
                                        config.junctionAccel);
    if (jv < p.exitVel) p.exitVel = jv;
  }

  // Limit point velocity
  p.entryVel = lastExitVel;
  if (p.maxVel < p.exitVel) p.exitVel = p.maxVel;

  // Plan & execute move
  points.push_back(p);
  points_t::iterator it = --points.end();
  if (plan(it)) backplan(it);

  lastExitVel = it->exitVel;
  lastUnit = it->unit;

  exec();
}


void LinePlanner::exec(const Point &p) {
  const double deltaTime = 0.005 / 60.0; // TODO this should be configurable
  double dist = 0;
  double vel = p.entryVel;
  double accel = 0;

  LOG_DEBUG(3, "Line:"
            << " length=" << setw(12) << p.length
            << " entryV=" << setw(12) << p.entryVel
            << " exitV="  << setw(12) << p.exitVel
            << " maxV="   << setw(12) << p.maxVel
            << " maxA="   << setw(12) << p.maxAccel
            << " maxJ="   << setw(12) << p.maxJerk);

  // Execute each S-curve segment
  for (int i = 0; i < 7; i++) {
    const double time = p.times[i];

    LOG_DEBUG(3, "Seg: #" << i << " time="  << setw(12) << time);

    // Jerk
    double jerk = 0;
    switch (i) {
    case 0: case 6: jerk = p.maxJerk; break;
    case 2: case 4: jerk = -p.maxJerk; break;
    }

    // Acceleration
    switch (i) {
    case 1: accel = p.maxJerk * p.times[0]; break;
    case 3: accel = 0; break;
    case 5: accel = -p.maxJerk * p.times[4]; break;
    }

    // TODO should be able to just look at time
    if (computeDistance(time, vel, accel, jerk) < 0.0001) continue;

    // Compute interpolation steps
    int steps = round(time / deltaTime);
    double deltaT = time / steps;

    if (time < deltaTime) {
      deltaT = time;
      steps = 1;
    }

    // Execute interpolation steps
    for (int j = 0; j < steps; j++) {
      double t = deltaT * (j + 1);
      double l = dist + computeDistance(t, vel, accel, jerk);
      double v = vel + computeVelocity(t, accel, jerk);

      cb::Vector4D pos = execPos + p.unit * l;
      Axes position;
      for (int k = 0; k < 4; k++) position[k] = pos[k];

      sink.move(deltaT, v, position);
    }

    // Update distance and velocity
    dist += computeDistance(time, vel, accel, jerk);
    vel += computeVelocity(time, accel, jerk);
  }

  if (vel < p.exitVel - 0.1 || p.exitVel + 0.1 < vel)
    THROWS("Velocity discontinuity: vel=" << vel << " exitVel=" << p.exitVel);

  // Keep track of last position
  execPos = p.position;
}


void LinePlanner::exec() {
  points_t::iterator it = points.begin();
  while (it != points.end())
    if (isFinal(it)) {
      exec(*it);
      points.pop_front();
      it = points.begin();
    } else break;
}


bool LinePlanner::isFinal(points_t::iterator it) const {
  double velocity = it->exitVel;
  if (velocity == 0) return true;

  // Check if there is enough velocity change in the following blocks to
  // deccelerate to zero if necessary.
  while (++it != points.end()) {
    velocity -= it->deltaV;
    if (velocity <= 0) return true;
  }

  return false;
}


bool LinePlanner::plan(points_t::iterator it) {
  Point &p = *it;
  bool backplan = false;
  double Vi = p.entryVel;
  double Vt = p.exitVel;

  bool swapped = false;
  if (Vt < Vi) {
    swap(Vi, Vt);
    swapped = true;
  }

  // Compute minimum length for velocity change
  double length = computeLength(Vi, Vt, p.maxAccel, p.maxJerk);

  // Check if velocity change fits
  if (p.length < length) {
    // Velocity change does not fit, compute a lower target velocity
    length = p.length; // New target velocity will fit exactly
    Vt = peakVelocity(Vi, p.maxAccel, p.maxJerk, length);

    // Update velocities
    if (swapped) {
      // Backplaning  necessary
      backplan = true;

      if (it == points.begin())
        THROWS("Cannot backplan, previous move unavailable");

      LOG_DEBUG(3, "Backplan: entryVel=" << p.entryVel
                << " prev.exitVel=" << prev(it)->exitVel << " Vt=" << Vt);

      p.entryVel = prev(it)->exitVel = Vt;

    } else {
      p.exitVel = Vt;
      if (next(it) != points.end()) next(it)->entryVel = Vt;
    }
  }

  // Zero times
  for (int i = 0; i < 7; i++) p.times[i] = 0;

  // Plan curve segments
  if ((0.95 * p.length <= length && length <= p.length) ||
      p.maxVel * 0.95 < Vt) {
    // Exact or near fit or target velocity is close to max, compute simple
    // velocity transition.
    double lengthRemain = p.length -
      planVelocityTransition(Vi, Vt, p.maxAccel, p.maxJerk, p.times);

    if (lengthRemain < -0.000001)
      THROWS("Velocity transition exceeds length by " << -lengthRemain
             << " required=" << p.length << " computed=" << length
             << " Vt=" << Vt);

    // If there is length left, add a constant velocity segment
    if (0.000001 < lengthRemain) p.times[3] = lengthRemain / Vt;

    // Record change in velocity
    p.deltaV = Vt - Vi;

  } else {
    // Velocity change fits and a higher peak velocity can be achieved.
    // Search for a peak velocity that fits well.
    double peakVel = p.maxVel;
    double maxVel = peakVel;
    double minVel = Vt;
    int rounds = 0;

    while (true) {
      double headLen = computeLength(Vi, peakVel, p.maxAccel, p.maxJerk);
      double tailLen = computeLength(Vt, peakVel, p.maxAccel, p.maxJerk);
      double bodyLen = p.length - headLen - tailLen;

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
    double length = p.length;
    length -=
      planVelocityTransition(Vi, peakVel, p.maxAccel, p.maxJerk, p.times);
    length -=
      planVelocityTransition(peakVel, Vt, p.maxAccel, p.maxJerk, p.times + 4);
    p.times[3] = length / peakVel;

    // Record change in velocity
    p.deltaV = peakVel - Vi + peakVel - Vt;
  }

  // Reverse the plan, if velocities were swapped above
  if (swapped)
    for (int i = 0; i < 3; i++)
      swap(p.times[i], p.times[6 - i]);

  return backplan;
}


void LinePlanner::backplan(points_t::iterator it) {
  while (true) {
    if (it == points.begin())
      THROWS("Cannot backplan, previous move unavailable");

    if (!plan(--it)) break;
  }
}
