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

#include "PlanCommand.h"

#include <math.h>

using namespace GCode;
using namespace std;


namespace {
  inline double square(double x) {return x * x;}
}


PlanMoveCommand::PlanMoveCommand(const PlannerConfig &config, const Axes &start,
                                 const Axes &end, double feed) :
  config(config), target(end), maxVel(feed) {

  // Compute direction unit vector
  dir = end - start;
  length = dir.length();
  dir = dir / length;

  // Compute max velocity
  for (unsigned i = 0; i < Axes::getSize(); i++)
    if (dir[i]) {
      double v = config.maxVel[i] / dir[i];
      if (v < maxVel) maxVel = v;
    }

  time = fabs(length / maxVel);
}


double PlanMoveCommand::computeJunctionVelocity(const Axes &dir) const {
  double cosTheta = dir.dot(this->dir);

  if (cosTheta < -0.99) return 10000000;  // straight line cases
  if (0.99 < cosTheta)  return 0;         // reversal cases

  // Fuse the junction deviations into a vector sum
  double aDelta = 0;
  double bDelta = 0;

  for (int axis = 0; axis < 9; axis++) {
    aDelta += square(dir[axis] * config.junctionDeviation);
    bDelta += square(this->dir[axis] * config.junctionDeviation);
  }

  if (!aDelta || !bDelta) return 0; // One of the direction unit vectors is null

  double delta = (sqrt(aDelta) + sqrt(bDelta)) / 2;
  double theta2 = acos(cosTheta) / 2;
  double radius = delta * sin(theta2) / (1 - sin(theta2));

  return sqrt(radius * config.junctionAccel);

}


double PlanMoveCommand::getMaxEntryVelocity(const Axes &dir) const {
  double velocity = min(maxVel, computeJunctionVelocity(dir));

  // Compute max axis velocity

  return velocity;
}
