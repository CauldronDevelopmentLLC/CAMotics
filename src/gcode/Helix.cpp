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

#include "Helix.h"

#include <cbang/Math.h>

using namespace GCode;
using namespace cb;
using namespace std;


Helix::Helix(const Vector3D &start, const Vector2D &centerOffset,
             const Vector3D &end, double angle, double maxError) :
  start(start), end(end), angle(angle),
  center(Vector2D(start.x() + centerOffset.x(), start.y() + centerOffset.y())) {
  // Compute offsets
  const double xOffset = centerOffset.x();
  const double yOffset = centerOffset.y();
  zOffset = end.z() - start.z();

  // Start angle
  startAngle = Vector2D(-xOffset, -yOffset).angleBetween(Vector2D(1, 0));

  // Radius
  radius = Vector2D(xOffset, yOffset).length();

  // Error cannot be greater than arc radius
  const double error = std::min(maxError, radius);

  // Compute segment angle from allowed error
  // See "Algorithm for circle approximation and generation" - L Yong-Kui.
  segAngle = 4 * atan(sqrt(error / radius));

  // Segment angle cannot be greater than 2Pi/3 because we need at least 3
  // segments in a full circle
  segAngle = std::min(2 * M_PI / 3, segAngle);

  // Compute integer number of segments that meets the error bound
  const unsigned segments = (unsigned)ceil(fabs(angle) / segAngle);
  points = segments + 2;

  // Adjusted segment angle
  segAngle = -angle / segments;

  // Increase the radius so the line segments straddle the actual helix
  double alpha = cos(segAngle / 2);
  double epsilon = radius * (1 - alpha) / (1 + alpha);
  radius += epsilon;

  // We want to start at a point where a line segment intersects the circle.
  // Find the angular offset to the next such point.
  const double beta = (M_PI - segAngle) / 2;
  offsetAngle = M_PI - beta - asin(radius * sin(beta) / (radius - epsilon));
}


Vector3D Helix::get(unsigned i) const {
  if (!i) return start;
  if (points - 1 <= i) return end;

  const double a = i * segAngle - offsetAngle;

  return Vector3D(center.x() + radius * cos(startAngle + a),
                  center.y() + radius * sin(startAngle + a),
                  start.z() + zOffset * a / -angle);
}
