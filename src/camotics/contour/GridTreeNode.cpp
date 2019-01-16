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

#include "GridTreeNode.h"
#include "GridTreeLeaf.h"

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


GridTreeNode::GridTreeNode(const Vector3U &steps) :
  left(0), right(0), count(0) {
  // Choose largest axis
  axis = 2;
  if (steps.y() <= steps.x() && steps.z() <= steps.x()) axis = 0;
  if (steps.x() <= steps.y() && steps.z() <= steps.y()) axis = 1;

  split = steps[axis] / 2;
}


GridTreeNode::~GridTreeNode() {
  if (left) delete left;
  if (right) delete right;
}


namespace {
  bool atLeaf(const Vector3U &steps) {
    return steps.x() == 1 && steps.y() == 1 && steps.z() == 1;
  }
}


void GridTreeNode::insertLeaf(GridTreeLeaf *leaf, const Vector3U &_steps,
                              const Vector3U &offset) {
  Vector3U steps(_steps);

  if (!steps.x() || !steps.y() || !steps.z()) THROW("Empty tree");

  // Left
  if (offset[axis] < split) {
    steps[axis] /= 2;

    if (atLeaf(steps)) {
      if (left) delete left;
      left = leaf;

    } else {
      if (!left) left = new GridTreeNode(steps);
      left->insertLeaf(leaf, steps, offset);
    }

  } else { // Right
    steps[axis] -= steps[axis] / 2;

    Vector3U rOffset(offset);
    rOffset[axis] -= split;

    if (atLeaf(steps)) {
      if (right) delete right;
      right = leaf;

    } else {
      if (!right) right = new GridTreeNode(steps);
      right->insertLeaf(leaf, steps, rOffset);
    }
  }

  count = (left ? left->getCount() : 0) + (right ? right->getCount() : 0);
}


void GridTreeNode::gather(vector<float> &vertices,
                          vector<float> &normals) const {
  if (left) left->gather(vertices, normals);
  if (right) right->gather(vertices, normals);
}
