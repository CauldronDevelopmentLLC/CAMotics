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

#include "GridTree.h"
#include "GridTreeRef.h"

#include <camotics/sim/AABBTree.h>

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


GridTree::GridTree(const Grid &grid) :
  GridTreeNode(grid.getSteps()), Grid(grid) {}


GridTree::~GridTree() {}


void GridTree::partition(vector<GridTreeRef> &grids,
                         const Rectangle3D &bbox, unsigned count) {
  Rectangle3D bounds = getBounds();
  if (isEmpty() || !bbox.intersects(bounds)) return;

  if (count < 2) {
    Vector3U offset;
    Vector3U steps(getSteps());

    Rectangle3D intersection = bbox.intersection(bounds);
    if (intersection != bounds) {
      intersection = (intersection - getOffset()) / getResolution();

      Rectangle3U iBounds(intersection.rmin.floor(),
                          intersection.rmax.ceil());
      offset = iBounds.rmin;
      Vector3U dims = iBounds.getDimensions();

      // Don't let new steps exceed old
      for (unsigned i = 0; i < 3; i++)
        if (steps[i] < offset[i] + dims[i]) steps[i] -= offset[i];
        else steps[i] = dims[i];
    }

    grids.push_back(GridTreeRef(this, offset, steps));
    return;
  }

  pair<Grid, Grid> parts = Grid::split(largestDim());

  if (!left) left = new GridTree(parts.first);
  if (!right) right = new GridTree(parts.second);

  dynamic_cast<GridTree *>(left)->partition(grids, bbox, count / 2);
  dynamic_cast<GridTree *>(right)->partition(grids, bbox, count - count / 2);
}


void GridTree::insertLeaf(GridTreeLeaf *leaf, const Vector3U &offset) {
  GridTreeNode::insertLeaf(leaf, getSteps(), offset);
}
