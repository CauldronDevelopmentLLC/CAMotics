/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "ConicSweep.h"

#include <cbang/log/Logger.h>

#include <limits>

using namespace std;
using namespace CAMotics;


ConicSweep::ConicSweep(real length, real radius1, real radius2) :
  l(length), rt(radius1), rb(radius2 == -1 ? radius1 : radius2),
  Tm((rt - rb) / l) {
}


void ConicSweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                           vector<Rectangle3R> &bboxes, real tolerance) const {
  Sweep::getBBoxes(start, end, bboxes, rt < rb ? rb : rt, l, tolerance);
}


namespace {
  inline double sqr(double x) {return x * x;}
}


real ConicSweep::depth(const Vector3R &start, const Vector3R &end,
                       const Vector3R &p) const {
  const double Ax = start.x(), Ay = start.y(), Az = start.z();
  const double Bx = end.x(), By = end.y(), Bz = end.z();
  const double Px = p.x(), Py = p.y(), Pz = p.z();

  // Check z-height
  const double minZ = Az < Bz ? Az : Bz, maxZ = Az < Bz ? Bz : Az;
  if (Pz < minZ || maxZ + l < Pz) return -1;

  const double a = Px - Ax, b = Py - Ay, c = (Pz - Az) * Tm;
  const double d = Bx - Ax, e = By - Ay, f = (Bz - Az) * Tm;
  const double a2 = a * a, b2 = b * b, c2 = c * c;
  const double d2 = d * d, e2 = e * e, f2 = f * f;
  const double rb2 = rb * rb;

  const double betaD = -d2 - e2 + f2;
  const double betaR =
    (d2 + e2) * rb2 +
    (2 * c * d2 + 2 * c * e2 + (-2 * b * e - 2 * a * d) * f) * rb +
    (a2 + b2) * f2 + (-2 * a * c * d - 2 * b * c * e) * f + (c2 - a2) * e2 +
    2 * a * b * d * e + (c2 - b2) * d2;

  // Check if solution is valid
  if (betaD == 0 || betaR < 0) return -1;

  double beta = (sqrt(betaR) + f * rb + c * f - b * e - a * d) / betaD;

  // Check if z-heights make sense
  const double Qz = beta * (Bz - Az) + Az;

  if (Pz < Qz || Qz + l < Pz) {
    beta = Pz / (Bz - Az); // E is on to AB at z-height

    // Compute squared distance to E on XY plane
    const double Ex = beta * (Bx - Ax) + Ax;
    const double Ey = beta * (By - Ay) + Ay;
    const double d2 = sqr(Ex - Px) + sqr(Ey - Py);

    if (Pz < Qz && rb2 < d2) return -1;
    if (Qz + l < Pz && rt * rt < d2) return -1;
  }

  // Check that it's on the line segment
  if (beta < 0 || 1 < beta) return -1;

  return 1;
}
