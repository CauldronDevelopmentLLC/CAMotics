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

#include "SpheroidSweep.h"

#include "Move.h"

#include <algorithm>

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
  Sweep::getBBoxes(start, end, bboxes, radius, length, -radius, tolerance);
}


namespace {
  inline double sqr(double x) {return x * x;}
}


real SpheroidSweep::depth(const Vector3R &_A, const Vector3R &_B,
                          const Vector3R &_P) const {
  const double r = radius;

  Vector3R A = _A;
  Vector3R B = _B;
  Vector3R P = _P;

  // Handle oblong spheroids by scaling the z-axis
  if (2 * radius != length) {
    A *= scale;
    B *= scale;
    P *= scale;
  }

  // Check z-height
  if (P.z() < min(A.z(), B.z()) || max(A.z(), B.z()) + 2 * r < P.z())
    return -1;

  const Vector3R AB = B - A;
  const Vector3R PA = A - P;

  // epsilon * beta^2 + gamma * beta + rho = 0
  const double epsilon = AB.dot(AB);
  const double gamma = AB.dot(PA + Vector3R(0, 0, r));
  const double rho = PA.dot(PA) + 2 * r * (A.z() - P.z());
  const double sigma = sqr(gamma) - epsilon * rho;

  // Check if solution is valid
  if (epsilon == 0 || sigma < 0) return -1;

  const double beta = (-gamma - sqrt(sigma)) / epsilon; // Quadradic equation

  // Check that it's on the line segment
  if (beta < 0 || 1 < beta) return -1;

  return 1;
}
