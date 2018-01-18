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

#include "Grid.h"

using namespace std;
using namespace cb;
using namespace CAMotics;


Grid::Grid(const cb::Rectangle3D &bounds, double resolution) :
  resolution(resolution), steps((bounds.getDimensions() / resolution).ceil()) {

  cb::Vector3D dims((cb::Vector3D)steps * resolution);
  offset = bounds.rmin - cb::Vector3D(dims - bounds.getDimensions()) / 2;
}


Grid::Grid(const cb::Vector3D &offset, const cb::Vector3U &steps,
           double resolution) :
  offset(offset), resolution(resolution), steps(steps) {}


cb::Rectangle3D Grid::getBounds() const {
  return cb::Rectangle3D(offset, offset + (cb::Vector3D)steps * resolution);
}


unsigned Grid::getTotalCells() const {
  return steps.x() * steps.y() * steps.z();
}


Grid Grid::slice(const cb::Vector3U &start) const {
  return
    Grid(offset + (cb::Vector3D)start * resolution, steps - start, resolution);
}


Grid Grid::slice(const cb::Vector3U &start, const cb::Vector3U &end) const {
  return
    Grid(offset + (cb::Vector3D)start * resolution, end - start, resolution);
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
  cb::Vector3U stepOffset;
  stepOffset[axis] = steps[axis] / 2;
  result.first = Grid(offset, steps - stepOffset, resolution);

  // Right
  cb::Vector3D rOffset(offset + (cb::Vector3D)stepOffset * resolution);
  stepOffset = steps - result.first.getSteps();
  result.second = Grid(rOffset, steps - stepOffset, resolution);

  return result;
}
