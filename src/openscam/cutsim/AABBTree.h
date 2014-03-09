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

#ifndef OPENSCAM_AABBTREE_H
#define OPENSCAM_AABBTREE_H

#include "AABB.h"

#include <openscam/Geom.h>
#include <openscam/cutsim/Move.h>

#include <vector>

namespace OpenSCAM {
  class ToolPath;

  class AABBTree {
  protected:
    AABB *root;

  public:
    AABBTree(AABB *nodes = 0);
    ~AABBTree();

    void partition(AABB *nodes);

    Rectangle3R getBounds() const;
    unsigned getLeafCount() const {return root ? root->getLeafCount() : 0;}
    unsigned getHeight() const {return root ? root->getTreeHeight() : 0;}

    bool intersects(const Rectangle3R &bbox) const;
    void collisions(const Vector3R &p, real time,
                    std::vector<const Move *> &moves);
    void draw(bool leavesOnly = false);
  };
}

#endif // OPENSCAM_AABBTREE_H

