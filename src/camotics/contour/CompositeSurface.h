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


#include "Surface.h"

#include <cbang/SmartPointer.h>

#include <vector>

namespace CAMotics {
  class CompositeSurface : public Surface {
    typedef std::vector<cb::SmartPointer<Surface> > surfaces_t;
    surfaces_t surfaces;

  public:
    void add(cb::SmartPointer<Surface> s);

    typedef surfaces_t::const_iterator iterator;
    iterator begin() const {return surfaces.begin();}
    iterator end() const {return surfaces.end();}

    cb::SmartPointer<Surface> consolidate();

    // From Surface
    cb::SmartPointer<Surface> copy() const;
    uint64_t getTriangleCount() const;
    cb::Rectangle3D getBounds() const;
    void getVertices(vert_cb_t cb) const;
    void write(STL::Sink &sink, Task *task = 0) const;
    void reduce(Task &task);

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
