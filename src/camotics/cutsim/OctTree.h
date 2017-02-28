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

#ifndef CAMOTICS_OCT_TREE_H
#define CAMOTICS_OCT_TREE_H

#include "MoveLookup.h"

#include <set>


namespace CAMotics {
  class OctTree : public MoveLookup {
    Rectangle3R bbox;

    class OctNode {
      Rectangle3R bounds;
      unsigned depth;

      OctNode *children[8];
      std::set<const Move *> moves;

    public:
      OctNode(const Rectangle3R &bounds, unsigned depth);
      ~OctNode();

      void insert(const Move *move, const Rectangle3R &bbox);
      bool intersects(const Rectangle3R &r) const;
      void collisions(const Vector3R &p,
                      std::vector<const Move *> &moves) const;
    };

    OctNode *root;

  public:
    OctTree(const Rectangle3R &bounds, unsigned depth);
    ~OctTree();

    // From MoveLookup
    Rectangle3R getBounds() const {return bbox;}
    void insert(const Move *move, const Rectangle3R &bbox);
    bool intersects(const Rectangle3R &r) const;
    void collisions(const Vector3R &p, std::vector<const Move *> &moves) const;
  };
}

#endif // CAMOTICS_OCT_TREE_H
