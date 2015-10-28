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

#include "CubeSlice.h"

#include <cbang/log/Logger.h>

using namespace std;
using namespace CAMotics;


CubeSlice::CubeSlice(const Rectangle3R &bbox, real maxStep) :
  bbox(bbox), maxStep(maxStep), shifted(false) {

  // Compute steps and step
  Rectangle2R bbox2 = bbox.slice<2>();
  steps = (bbox2.getDimensions() / maxStep).ceil();
  step = bbox2.getDimensions() / steps;
}


void CubeSlice::compute(FieldFunction &func) {
  // Vertices
  if (!shifted) {
    left = new VertexSlice(bbox.slice<2>(), maxStep, bbox.getMin().z());
    left->compute(func);
  }

  right = new VertexSlice(bbox.slice<2>(), maxStep, bbox.getMax().z());
  right->compute(func);

  // Edges
  for (unsigned i = 0; i < 5; i++)
    edges[i].resize(steps.x() + 1, vector<Edge>(steps.y() + 1));

  Vector3R p = bbox.rmin;
  static int vIndex[5][3] = {
    {1, 0, 0}, // a
    {0, 1, 0}, // b
    {0, 0, 1}, // c
    {1, 0, 1}, // d
    {0, 1, 1}, // e
  };
  Vector3R offset(step.x(), step.y(), bbox.getDimensions().z());

  Vector3R offsets[5];
  for (unsigned i = 0; i < 5; i++)
    offsets[i] = offset *
      Vector3R(vIndex[i][0], vIndex[i][1], vIndex[i][2]);

  // For each cell
  for (unsigned x = 0; x <= steps.x(); x++) {
    p.x() = bbox.rmin.x() + step.x() * x;

    for (unsigned y = 0; y <= steps.y(); y++) {
      p.y() = bbox.rmin.y() + step.y() * y;

      Vector3R a = p;
      bool aInside = left->isInside(x, y);

      for (unsigned i = shifted ? 2 : 0; i < 5; i++) {
        if (x == steps.x() && (i == 0 || i == 3)) continue;
        if (y == steps.y() && (i == 1 || i == 4)) continue;

        Vector3R b = p + offsets[i];
        bool bInside = isInside(x, y, vIndex[i]);

        if (aInside != bInside)
          edges[i][x][y] = func.getEdge(a, aInside, b, bInside);

        if (i == 2) {
          a = b;
          aInside = bInside;
        }
      }
    }
  }
}


void CubeSlice::shiftZ(real z) {
  // Vertices
  left = right;
  right.release();

  // Edges
  edges[0] = edges[3];
  edges[1] = edges[4];

  // Bounds
  real deltaZ = bbox.getDimensions().z();
  bbox.rmin.z() = z;
  bbox.rmax.z() = z + deltaZ;

  shifted = true;
}


uint8_t CubeSlice::getEdges(unsigned x, unsigned y, Edge edges[12]) const {
  // This table maps vertices to the index needed for the marching cubes table.
  static int vOffset[8][3] = {
    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1},
  };

  uint8_t index = 0;
  for (unsigned i = 0; i < 8; i++)
    if (!isInside(x, y, vOffset[i]))
      index |= 1 << i;

  // eOffset[] maps cube indices to the 12 edges and edge flags as laid out in
  // the marching cubes tables.
  //
  //  Edge  |      X, Y offsets
  // offset | 0, 0  1, 0  0, 1  1, 1
  // -------+-----------------------
  //   0    | 0->1  ----  3->2  ----
  //   1    | 0->3  1->2  ----  ----
  //   2    | 0->4  1->5  3->7  2->6
  //   3    | 4->5  ----  7->6  ----
  //   4    | 4->7  5->6  ----  ----
  //
  // Marching cubes edges
  //  0->1  1->2  2->3  3->0
  //  4->5  5->6  6->7  7->4
  //  0->4  1->5  2->6  3->7
  //
  static int eOffset[12][3] = {
    {0, 0, 0}, {1, 0, 1}, {0, 1, 0}, {0, 0, 1},
    {0, 0, 3}, {1, 0, 4}, {0, 1, 3}, {0, 0, 4},
    {0, 0, 2}, {1, 0, 2}, {1, 1, 2}, {0, 1, 2},
  };

  static const int edgeConnection[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
  };

  for (unsigned i = 0; i < 12; i++)
    if (!(index & (1 << edgeConnection[i][0])) !=
        !(index & (1 << edgeConnection[i][1]))) {
      edges[i] =
        this->edges[eOffset[i][2]][x + eOffset[i][0]][y + eOffset[i][1]];

      //LOG_DEBUG(1, "i=" << i << " edge=" << edges[i].vertex << " x=" << x
      //          << " y=" << y);
    }

  return index;
}


bool CubeSlice::isInside(int x, int y, int offset[3]) const {
  return (offset[2] ? right : left)->isInside(x + offset[0], y + offset[1]);
}
