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

#include "Grid.h"

using namespace std;
using namespace cb;
using namespace CAMotics;


Grid::Grid(const Rectangle3D &bounds, double resolution) :
  resolution(resolution), steps((bounds.getDimensions() / resolution).ceil()) {

  Vector3D dims((Vector3D)steps * resolution);
  offset = bounds.rmin - Vector3D(dims - bounds.getDimensions()) / 2;
  origin = offset;
}


Grid::Grid(const Vector3D &offset, const Vector3U &steps,
           double resolution) :
  offset(offset), resolution(resolution), steps(steps), origin(offset) {}


void Grid::setLattice(const Vector3D &origin, const Vector3U &index) {
  this->origin = origin;
  this->index = index;
  offset = point(0, 0, 0);
}


Rectangle3D Grid::getBounds() const {
  return Rectangle3D(offset, offset + (Vector3D)steps * resolution);
}


unsigned Grid::getTotalCells() const {
  return steps.x() * steps.y() * steps.z();
}


Grid Grid::slice(const Vector3U &start) const {
  return slice(start, steps);
}


Grid Grid::slice(const Vector3U &start, const Vector3U &end) const {
  Grid grid(offset, end - start, resolution);
  grid.setLattice(origin, index + start);
  return grid;
}


unsigned Grid::largestDim() const {
  if (steps.y() <= steps.x() && steps.z() <= steps.x()) return 0;
  if (steps.x() <= steps.y() && steps.z() <= steps.y()) return 1;
  return 2;
}


void Grid::partition(vector<Grid> &grids, unsigned count) const {
  if (isEmpty()) return;

  if (count < 2) {
    grids.push_back(*this);
    return;
  }

  // Partition on the largest dimension
  pair<Grid, Grid> parts = split(largestDim());

  unsigned half = count / 2;
  parts.first.partition(grids, half);
  parts.second.partition(grids, count - half);
}


pair<Grid, Grid> Grid::split(unsigned axis) const {
  pair<Grid, Grid> result(*this, Grid());

  if (steps[axis] < 2) return result;

  // Left
  Vector3U stepOffset;
  stepOffset[axis] = steps[axis] / 2;
  result.first = Grid(offset, steps - stepOffset, resolution);
  result.first.setLattice(origin, index);

  // Right
  Vector3U rIndex(index + stepOffset);
  stepOffset = steps - result.first.getSteps();
  result.second = Grid(offset, steps - stepOffset, resolution);
  result.second.setLattice(origin, rIndex);

  return result;
}
