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

#include "Grid.h"

using namespace std;
using namespace cb;
using namespace CAMotics;


Grid::Grid(const Rectangle3D &bounds, double resolution) :
  resolution(resolution), steps((bounds.getDimensions() / resolution).ceil()) {

  Vector3D dims((Vector3D)steps * resolution);

  rmin = bounds.rmin - Vector3D(dims - bounds.getDimensions()) / 2;
  rmax = rmin + dims;
}


Grid::Grid(const Vector3D &rmin, const Vector3U &steps, double resolution) :
  Rectangle3D(rmin, rmin + (Vector3D)steps * resolution),
  resolution(resolution), steps(steps) {}


unsigned Grid::getTotalCells() const {
  return steps.x() * steps.y() * steps.z();
}


Grid Grid::slice(const Vector3U &start) const {
  return Grid(rmin + (Vector3D)start * resolution, steps - start, resolution);
}


Grid Grid::slice(const Vector3U &start, const Vector3U &end) const {
  return Grid(rmin + (Vector3D)start * resolution, end - start, resolution);
}


void Grid::partition(vector<Grid> &grids, unsigned count) const {
  if (isEmpty()) return;

  if (count < 2) {
    grids.push_back(*this);
    return;
  }

  Vector3U offset;

  // Partition on the largest dimension
  if (getLength() <= getWidth() && getHeight() <= getWidth())
    offset.x() = steps.x() / 2;
  else if (getWidth() <= getLength() && getHeight() <= getLength())
    offset.y() = steps.y() / 2;
  else offset.z() = steps.z() / 2;

  Grid left(rmin, steps - offset, resolution);
  Vector3D rightMin(rmin + (Vector3D)offset * resolution);
  offset = steps - left.getSteps();
  Grid right(rightMin, steps - offset, resolution);

  left.partition(grids, count / 2);
  right.partition(grids, count / 2);
}
