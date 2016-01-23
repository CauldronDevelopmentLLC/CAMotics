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

#include "AABBTree.h"

#include <cbang/Zap.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


AABBTree::~AABBTree() {
  zap(root);
}


Rectangle3R AABBTree::getBounds() const {
  if (!finalized) THROWS("AABBTree not yet finalized");
  return root ? root->getBounds() : Rectangle3R();
}


void AABBTree::insert(const Move *move, const Rectangle3R &bbox) {
  if (finalized) THROWS("Cannot insert into AABBTree after partitioning");
  root = (new AABB(move, bbox))->prepend(root);
}


bool AABBTree::intersects(const Rectangle3R &r) const {
  if (!finalized) THROWS("AABBTree not yet finalized");
  return root && root->intersects(r);
}


void AABBTree::collisions(const Vector3R &p,
                          vector<const Move *> &moves) const {
  if (!finalized) THROWS("AABBTree not yet finalized");
  if (root) root->collisions(p, moves);
}


void AABBTree::draw(bool leavesOnly) {
  if (root) root->draw(leavesOnly, root->getTreeHeight());
}


void AABBTree::finalize() {
  if (finalized) return;
  finalized = true;
  root = new AABB(root);
}
