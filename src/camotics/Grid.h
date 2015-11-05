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

#ifndef CAMOTICS_GRID_H
#define CAMOTICS_GRID_H

#include "Geom.h"

#include <vector>


namespace CAMotics {
  class Grid : public cb::Rectangle3D {
    double resolution;
    cb::Vector3U steps;

  public:
    Grid(const cb::Rectangle3D &bounds, double resolution);
    Grid(const cb::Vector3D &rmin, const cb::Vector3U &steps,
         double resolution);

    double getResolution() const {return resolution;}
    const cb::Vector3U getSteps() const {return steps;}
    unsigned getTotalCells() const;

    Grid slice(const cb::Vector3U &start) const;
    Grid slice(const cb::Vector3U &start, const cb::Vector3U &end) const;

    void partition(std::vector<Grid> &grids, unsigned count) const;
  };
}

#endif // CAMOTICS_GRID_H

