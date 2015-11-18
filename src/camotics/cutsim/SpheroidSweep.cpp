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

#include "SpheroidSweep.h"

#include "Move.h"

using namespace std;
using namespace CAMotics;


SpheroidSweep::SpheroidSweep(real radius, real length) :
  radius(radius), length(length == -1 ? radius : length) {

  if (2 * radius != length) scale = Vector3R(1, 1, 2 * radius / length);
  radius2 = radius * radius;
}


void SpheroidSweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                              vector<Rectangle3R> &bboxes,
                              real tolerance) const {
  Sweep::getBBoxes(start, end, bboxes, radius, length, tolerance);
}


namespace {
  inline double sqr(double x) {return x * x;}
}


real SpheroidSweep::depth(const Vector3R &_start, const Vector3R &_end,
                          const Vector3R &_p) const {
  const double r = radius;

  Vector3R start = _start;
  Vector3R end = _end;
  Vector3R p = _p;

  if (2 * radius != length) {
    // TODO this is not quite right
    start *= scale;
    end *= scale;
    p *= scale;
  }

  const double Ax = start.x(), Ay = start.y(), Az = start.z();
  const double Bx = end.x(), By = end.y(), Bz = end.z();
  const double Px = p.x(), Py = p.y(), Pz = p.z();

  // Check z-height
  const double minZ = Az < Bz ? Az : Bz, maxZ = Az < Bz ? Bz : Az;
  if (Pz < minZ || maxZ + 2 * r < Pz) return -1;

  // Squares
  const double r2 = r * r;

  const double a = Px - Ax, b = Py - Ay, c = Pz - Az - r;
  const double d = Bx - Ax, e = By - Ay, f = Bz - Az;
  const double a2 = a * a, b2 = b * b, c2 = c * c;
  const double d2 = d * d, e2 = e * e, f2 = f * f;

  const double betaD = d2 + e2 + f2;
  const double betaR =
    betaD * r2 + (-a2 - b2) * f2 + (2 * a * c * d + 2 * b * c * e) * f +
    (-a2 - c2) * e2 + 2 * a * b * d * e + (-b2 - c2) * d2;

  // Check if solution is valid
  if (betaD == 0 || betaR < 0) return -1;

  double beta = (sqrt(betaR) + c * f + b * e + a * d) / betaD;

  // Check that it's on the line segment
  if (beta < 0 || 1 < beta) return -1;

  return 1;
}
