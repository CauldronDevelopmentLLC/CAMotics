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

#include "AABB.h"

#include <openscam/view/GL.h>
#include <openscam/view/BoundsView.h>

#include <cbang/Zap.h>

#include <algorithm>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


/// NOTE: Expects @param nodes to be a link list along the left child
AABB::AABB(AABB *nodes) : left(0), right(0), move(0) {
  if (!nodes) return;

  // Compute bounds
  unsigned count = 0;
  for (AABB *it = nodes; it; it = it->left) {
    if (it->right) THROW("Unexpected right-hand AABB node");
    add(*it);
    count++;
  }

  // Degenerate cases
  if (count < 3) {
    if (count == 2) right = nodes->left;
    left = nodes->prepend(0);
    return;
  }

  // Decide split
  unsigned axis = getDimensions().findLargest();
  real cut = getMax()[axis] + getMin()[axis];

  // Partition nodes
  AABB *lessThan = 0;
  AABB *greaterThan = 0;
  unsigned lessCount = 0;
  unsigned greaterCount = 0;

  for (AABB *it = nodes; it;) {
    AABB *next = it->left;
    bool less = it->getMax()[axis] + it->getMin()[axis] < cut;

    if (less) {lessThan = it->prepend(lessThan); lessCount++;}
    else {greaterThan = it->prepend(greaterThan); greaterCount++;}

    it = next;
  }

  // Check for bad partition
  if (!lessThan) lessThan = greaterThan->split(greaterCount / 2);
  if (!greaterThan) greaterThan = lessThan->split(lessCount / 2);

  // Recur
  left = new AABB(lessThan);
  right = new AABB(greaterThan);
}


AABB::~AABB() {
  zap(left);
  zap(right);
}


AABB *AABB::prepend(AABB *list) {
  left = list;
  return this;
}


AABB *AABB::split(unsigned count) {
  if (!count) return this;
  if (count != 1) return left ? left->split(count - 1) : 0;
  AABB *tmp = left;
  left = 0;
  return tmp;
}


unsigned AABB::getTreeHeight() const {
  return max(left ? left->getTreeHeight() : 0,
             right ? right->getTreeHeight() : 0) + 1;
}


unsigned AABB::getLeafCount() const {
  return
    (left ? left->getLeafCount() : 0) +
    (right ? right->getLeafCount() : 0) +
    (move ? 1 : 0);
}


void AABB::collisions(const Vector3R &p, real time,
                      vector<const Move *> &moves) {
  if (!contains(p)) return;
  if (move && move->getStartTime() <= time) moves.push_back(move);
  if (left) left->collisions(p, time, moves);
  if (right) right->collisions(p, time, moves);
}


void AABB::draw(bool leavesOnly, unsigned height, unsigned depth) {
  if (!(left || right) || !leavesOnly) {
    glColor4f(0.5, 0, (height - depth) / (real)height, 1);
    BoundsView(*this).draw();
  }

  if (left) left->draw(leavesOnly, height, depth + 1);
  if (right) right->draw(leavesOnly, height, depth + 1);
}
