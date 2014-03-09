/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

using namespace std;
using namespace OpenSCAM;


ConicSweep::ConicSweep(real length, real radius1, real radius2) :
  length(length), radius1(radius1), radius2(radius2 == -1 ? radius1 : radius2) {
}


void ConicSweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                           vector<Rectangle3R> &bboxes, real tolerance) const {
  Sweep::getBBoxes(start, end, bboxes, radius1 < radius2 ? radius2 : radius1,
                   length, tolerance);
}



bool ConicSweep::contains(const Vector3R &start, const Vector3R &end,
                          const Vector3R &p) const {
  real x1 = start.x();
  real y1 = start.y();
  real z1 = start.z();

  real x2 = end.x();
  real y2 = end.y();
  real z2 = end.z();

  real x = p.x();
  real y = p.y();
  real z = p.z();

  // Z height range
  real minZ = z1 < z2 ? z1 : z2;
  real maxZ = z1 < z2 ? z2 : z1;
  if (z < minZ || maxZ + length < z) return false;

  real xLen = x2 - x1;
  real yLen = y2 - y1;
  real zLen = z2 - z1;

  bool horizontal = z1 == z2;
  bool vertical = x1 == x2 && y1 == y2;
  bool conical = radius1 != radius2;

  Vector2R q(x, y);
  Vector2R p1(x1, y1);
  Vector2R p2(x2, y2);

  real conicSlope = conical ? (radius1 - radius2) / length : 0;

  // Simple cases for horizontal and vertical moves
  real r2;
  if (conical) {
    if (z <= minZ) r2 = radius2;
    else if (maxZ + length <= z) r2 = radius1;
    else r2 = (z - minZ) * conicSlope + radius2;
    r2 *= r2;

  } else r2 = radius1 * radius1;

  if (vertical) return p1.distanceSquared(q) < r2;

  Vector2R c = Segment2R(p1, p2).closest(q);
  if (horizontal) return q.distanceSquared(c) < r2;

  // Slanting move
  // Find the ends of the line segment which passes through the move's
  // internal parallelogram at the z-height of p
  int count = 0;
  Vector2R s[2];

  // First check the verticals
  if (z1 <= z && z <= z1 + length) s[count++] = p1;
  if (z2 <= z && z <= z2 + length) s[count++] = p2;

  // Check top & bottom lines of the parallelogram
  if (count < 2) {
    real delta;
    if (minZ + length < z) delta = (z - minZ + length) / std::fabs(z2 - z1);
    else delta = (z - minZ) / std::fabs(z2 - z1);
    if (z1 < z2) s[count++] = p1 + (p2 - p1) * delta;
    else s[count++] = p2 + (p1 - p2) * delta;
  }

  // Find the closest point to p on the z-line segment
  c = Segment2R(s[0], s[1]).closest(q);

  // Check cone tool height for closest point
  if (conical) {
    // NOTE Slanting move so one or both of xLen and yLen is not zero
    real h = 0;
    if (xLen) h = z - (z1 + (c.x() - x1) / xLen * zLen);
    else if (yLen) h = z - (z1 + (c.y() - y1) / yLen * zLen);

    if (h < length) {
      r2 = h * conicSlope + radius2;
      r2 *= r2;
    }
  }

  real cDistSq = q.distanceSquared(c);
  if (cDistSq < r2) return true;
  if (!conical) return false;

  // Look up and down the z-line for a closer point on the cone
  real maxRadius = radius1 < radius2 ? radius2 : radius1;
  real l2 = maxRadius * maxRadius - cDistSq;
  real l = sqrt(l2); // How far to look
  Vector2R n = (s[1] - s[0]).normalize(); // Direction vector

  Vector2R a = c.distanceSquared(s[1]) < l2 ? s[1] : (c + n * l);
  Vector2R b = c.distanceSquared(s[0]) < l2 ? s[0] : (c - n * l);
  real aH = 0;
  real bH = 0;

  if (xLen) {
    aH = z - (z1 + (a.x() - x1) / xLen * zLen);
    bH = z - (z1 + (b.x() - x1) / xLen * zLen);

  } else if (yLen) {
    aH = z - (z1 + (a.y() - y1) / yLen * zLen);
    bH = z - (z1 + (b.y() - y1) / yLen * zLen);
  }

  real h = aH < bH ? bH : aH;
  c = aH < bH ? b : a;
  r2 = h * conicSlope + radius2;
  r2 *= r2;

  return q.distanceSquared(c) < r2;
}
