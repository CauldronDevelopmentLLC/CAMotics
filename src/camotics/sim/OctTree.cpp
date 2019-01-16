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

#include "OctTree.h"

#include <cbang/Zap.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


OctTree::OctNode::OctNode(const Rectangle3D &bounds, unsigned depth) :
  bounds(bounds), depth(depth) {
  for (int i = 0; i < 8; i++) children[i] = 0;
}


OctTree::OctNode::~OctNode() {
  for (int i = 0; i < 8; i++)
    if (children[i]) delete children[i];
}


void OctTree::OctNode::insert(const GCode::Move *move,
                              const Rectangle3D &bbox) {
  if (!bounds.intersects(bbox)) return;

  if (!depth || bbox.contains(bounds)) {
    moves.insert(move);
    return;
  }

  Vector3D dims = bounds.getDimensions();

  static Vector3D cubes[16] = {
    Vector3D(0.0, 0.0, 0.0), Vector3D(0.5, 0.5, 0.5),
    Vector3D(0.5, 0.0, 0.0), Vector3D(1.0, 0.5, 0.5),
    Vector3D(0.0, 0.5, 0.0), Vector3D(0.5, 1.0, 0.5),
    Vector3D(0.5, 0.5, 0.0), Vector3D(1.0, 1.0, 0.5),

    Vector3D(0.0, 0.0, 0.5), Vector3D(0.5, 0.5, 1.0),
    Vector3D(0.5, 0.0, 0.5), Vector3D(1.0, 0.5, 1.0),
    Vector3D(0.0, 0.5, 0.5), Vector3D(0.5, 1.0, 1.0),
    Vector3D(0.5, 0.5, 0.5), Vector3D(1.0, 1.0, 1.0),
  };

  for (int i = 0; i < 8; i++) {
    if (!children[i]) {
      Rectangle3D cBounds(bounds.getMin() + dims * cubes[i * 2],
                          bounds.getMin() + dims * cubes[i * 2 + 1]);
      children[i] = new OctNode(cBounds, depth - 1);
    }

    children[i]->insert(move, bbox);
  }
}


bool OctTree::OctNode::intersects(const Rectangle3D &r) const {
  if (!bounds.intersects(r)) return false;
  if (!moves.empty()) return true;

  if (depth)
    for (int i = 0; i < 8; i++)
      if (children[i] && children[i]->intersects(r))
        return true;

  return false;
}


void OctTree::OctNode::collisions(const Vector3D &p,
                                  vector<const GCode::Move *> &moves) const {
  if (!bounds.contains(p)) return;

  moves.insert(moves.end(), this->moves.begin(), this->moves.end());

  if (depth)
    for (int i = 0; i < 8; i++)
      if (children[i]) children[i]->collisions(p, moves);
}


OctTree::OctTree(const Rectangle3D &bounds, unsigned depth) {
  double m = bounds.getDimensions().max();

  root = new OctNode(Rectangle3D(bounds.getMin(), bounds.getMin() +
                                 Vector3D(m, m, m)), depth);
}


OctTree::~OctTree() {
  zap(root);
}


void OctTree::insert(const GCode::Move *move, const Rectangle3D &bbox) {
  this->bbox.add(bbox);
  root->insert(move, bbox);
}


bool OctTree::intersects(const Rectangle3D &r) const {
  return root->intersects(r);
}


void OctTree::collisions(const Vector3D &p,
                         vector<const GCode::Move *> &moves) const {
  root->collisions(p, moves);
}
