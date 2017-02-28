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

#pragma once


#include "MoveLookup.h"
#include "AABB.h"

#include <camotics/Geom.h>
#include <camotics/cutsim/Move.h>

#include <vector>

namespace CAMotics {
  class ToolPath;

  class AABBTree : public MoveLookup {
  protected:
    AABB *root;
    bool finalized;

  public:
    AABBTree() : root(0), finalized(false) {}
    virtual ~AABBTree();

    unsigned getHeight() const {return root ? root->getTreeHeight() : 0;}

    // From MoveLookup
    Rectangle3R getBounds() const;
    void insert(const Move *move, const Rectangle3R &bbox);
    bool intersects(const Rectangle3R &r) const;
    void collisions(const Vector3R &p, std::vector<const Move *> &moves) const;
    void finalize();
    void draw(bool leavesOnly = false);
  };
}
