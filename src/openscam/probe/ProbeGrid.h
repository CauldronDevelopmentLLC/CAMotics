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

#ifndef OPENSCAM_PROBE_GRID_H
#define OPENSCAM_PROBE_GRID_H

#include "ProbePoint.h"

#include <openscam/Geom.h>

#include <cbang/SmartPointer.h>

#include <vector>

namespace OpenSCAM {
  class ProbeGrid : public std::vector<std::vector<ProbePoint> > {
    cb::Rectangle2D bbox;
    cb::Vector2D divisions;
    cb::Vector2D cellSize;

  public:
    typedef std::vector<ProbePoint> Column_t;
    typedef std::vector<Column_t> Row_t;
    typedef Row_t::iterator row_iterator;
    typedef Column_t::iterator col_iterator;
    typedef Column_t::reverse_iterator reverse_col_iterator;

    ProbeGrid(const cb::Rectangle2D &bbox, const cb::Vector2D &divisions);

    std::vector<ProbePoint *> find(const cb::Vector2D &p);
  };
}

#endif // OPENSCAM_PROBE_GRID_H

