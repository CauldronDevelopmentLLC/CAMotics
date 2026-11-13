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

#pragma once

#include <cbang/geom/Rectangle.h>

#include <vector>
#include <utility>


namespace CAMotics {
  class Grid {
    cb::Vector3D offset;
    double resolution;
    cb::Vector3U steps;

    // Grid points are computed as origin + resolution * (index + i) so that
    // grids split into separate render jobs compute bit-identical
    // coordinates on the planes they share.
    cb::Vector3D origin;
    cb::Vector3U index;

  public:
    Grid() : resolution(0) {}
    Grid(const cb::Rectangle3D &bounds, double resolution);
    Grid(const cb::Vector3D &offset, const cb::Vector3U &steps,
         double resolution);

    bool isEmpty() const {return !steps.x() || !steps.y() || !steps.z();}
    bool isLeaf() const
    {return steps.x() == 1 && steps.y() == 1 && steps.z() == 1;}

    const cb::Vector3D &getOffset() const {return offset;}
    void setOffset(const cb::Vector3D &offset) {this->offset = offset;}

    const cb::Vector3D &getOrigin() const {return origin;}
    const cb::Vector3U &getIndex() const {return index;}
    void setLattice(const cb::Vector3D &origin, const cb::Vector3U &index);

    double coord(unsigned axis, unsigned i) const
    {return origin[axis] + resolution * (double)(index[axis] + i);}
    cb::Vector3D point(unsigned x, unsigned y, unsigned z) const
    {return cb::Vector3D(coord(0, x), coord(1, y), coord(2, z));}

    double getResolution() const {return resolution;}
    void setResolution(double resolution) {this->resolution = resolution;}

    const cb::Vector3U &getSteps() const {return steps;}
    void setSteps(const cb::Vector3U steps) {this->steps = steps;}

    cb::Rectangle3D getBounds() const;
    unsigned getTotalCells() const;

    Grid slice(const cb::Vector3U &start) const;
    Grid slice(const cb::Vector3U &start, const cb::Vector3U &end) const;

    unsigned largestDim() const;
    void partition(std::vector<Grid> &grids, unsigned count) const;
    std::pair<Grid, Grid> split(unsigned axis) const;
  };
}
