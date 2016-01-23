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

#ifndef CAMOTICS_MOVE_LOOKUP_H
#define CAMOTICS_MOVE_LOOKUP_H

#include <camotics/Geom.h>
#include <camotics/cutsim/Move.h>

namespace CAMotics {
  class MoveLookup {
  public:
    virtual ~MoveLookup() {}

    virtual Rectangle3R getBounds() const = 0;
    virtual void insert(const Move *move, const Rectangle3R &bbox) = 0;
    virtual bool intersects(const Rectangle3R &r) const = 0;
    virtual void collisions(const Vector3R &p,
                            std::vector<const Move *> &moves) const = 0;
    virtual void finalize() {}
    virtual void draw(bool leavesOnly = false) {}
  };
}

#endif // CAMOTICS_MOVE_LOOKUP_H

