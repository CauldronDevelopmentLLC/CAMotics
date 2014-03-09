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

#include "AABBTree.h"

#include <cbang/Zap.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


AABBTree::AABBTree(AABB *nodes) : root(nodes ? new AABB(nodes) : 0) {}


AABBTree::~AABBTree() {
  zap(root);
}


void AABBTree::partition(AABB *nodes) {
  zap(root);
  root = new AABB(nodes);
}


Rectangle3R AABBTree::getBounds() const {
  return root ? root->getBounds() : Rectangle3R();
}


bool AABBTree::intersects(const Rectangle3R &bbox) const {
  return root ? root->intersects(bbox) : false;
}


void AABBTree::collisions(const Vector3R &p, real time,
                          vector<const Move *> &moves) {
  if (root) root->collisions(p, time, moves);
}


void AABBTree::draw(bool leavesOnly) {
  if (root) root->draw(leavesOnly, root->getTreeHeight());
}
