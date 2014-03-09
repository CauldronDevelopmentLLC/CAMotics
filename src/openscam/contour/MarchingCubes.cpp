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

#include "MarchingCubes.h"

#include <limits>

#include <cbang/Math.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;

// These tables are used so that everything can be done in little loops that
// you can look at all at once rather than in pages and pages of unrolled code.
// vertexOffset lists the positions, relative to vertex0, of each of the 8
// vertices of a cube
static const Vector3R vertexOffset[8] = {
  Vector3R(0, 0, 0), Vector3R(1, 0, 0), Vector3R(1, 1, 0), Vector3R(0, 1, 0),
  Vector3R(0, 0, 1), Vector3R(1, 0, 1), Vector3R(1, 1, 1), Vector3R(0, 1, 1),
};

extern const int triangleConnectionTable[256][16];
extern const int cubeEdgeFlags[256];
extern const int tetrahedronTriangles[16][7];
extern const int tetrahedronEdgeConnection[6][2];
extern const int tetrahedronEdgeFlags[16];


void MarchingCubes::run(FieldFunction &func, const Rectangle3R &bbox,
                        real step) {
  surface = new ElementSurface(3);

  // Compute steps
  Vector3U steps = bbox.getDimensions() / step;
  for (int i = 0; i < 3; i++) if (steps[i] < 2) steps[i] = 2;

  // Compute scaling
  Vector3R scale = bbox.getDimensions() / steps;

  // Progress
  completedCells = 0;
  totalCells = steps.x() * steps.y() * steps.z();

  // Recursive march
  march(func, bbox.getMin(), scale, steps);
}


void MarchingCubes::march(FieldFunction &func, const Vector3R &p,
                          const Vector3R &scale, const Vector3U &steps) {
  if (interrupt) return;

  if (steps[0] == 1 && steps[1] == 1 && steps[2] == 1) {
    Vector3R samplePt = p;

    if (tetrahedrons) marchTetrahedrons(func, samplePt, scale);
    else marchCube(func, samplePt, scale);

    // Progress
    completedCells++;
    updateProgress((double)completedCells / totalCells);

  } else {
    // Cull empty boxes
    if (4 < steps[0] + steps[1] + steps[2]) {
      Vector3R max = p + Vector3R(steps[0], steps[1], steps[2]) * scale;
      if (func.canCull(Rectangle3R(p, max))) return;
    }

    // Find largest dimension
    unsigned maxDim = steps.findLargest();

    // Divide the steps
    unsigned steps1[3];
    unsigned steps2[3];
    for (int i = 0; i < 3; i++) steps1[i] = steps2[i] = steps[i];
    steps1[maxDim] /= 2;
    steps2[maxDim] = steps[maxDim] - steps1[maxDim];

    // Find mid point
    Vector3R p2 = p;
    p2[maxDim] += steps1[maxDim] * scale[maxDim];

    // Recur
    march(func, p, scale, steps1);
    march(func, p2, scale, steps2);
  }
}


// marchCube() performs the Marching Cubes algorithm on a single cube
void MarchingCubes::marchCube(FieldFunction &func, const Vector3R &p,
                              const Vector3R &scale) {
  // edgeConnection lists the index of the endpoint vertices for each of the
  // 12 edges of the cube
  static const int edgeConnection[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
  };

  // Make a local copy of the status at the cube's corners
  bool cubeInside[8];
  for (int i = 0; i < 8; i++)
    cubeInside[i] = func.contains(p + vertexOffset[i] * scale);

  // Find which vertices are inside of the surface and which are outside
  int flagIndex = 0;
  for (int i = 0; i < 8; i++)
    if (!cubeInside[i]) flagIndex |= 1 << i;

  // Find which edges are intersected by the surface
  int edgeFlags = cubeEdgeFlags[flagIndex];

  // If the corners of the cube are all entirely inside or all outside of the
  // surface, then there will be no intersections
  if (!edgeFlags) return;

  // Find the point of intersection of the surface with each edge
  // Then find the normal to the surface at those points
  Vector3R edgeVertex[12];
  for (int i = 0; i < 12; i++) {
    // If there is an intersection on this edge
    if (edgeFlags & (1 << i)) {
      Vector3R v1 = p + vertexOffset[edgeConnection[i][0]] * scale;
      Vector3R v2 = p + vertexOffset[edgeConnection[i][1]] * scale;
      bool inside1 = cubeInside[edgeConnection[i][0]];
      bool inside2 = cubeInside[edgeConnection[i][1]];
      edgeVertex[i] = func.getEdgeIntersect(v1, inside1, v2, inside2);
    }
  }

  // Draw the triangles that were found. There can be up to five per cube
  for (int j = 0; j < 5; j++) {
    if (triangleConnectionTable[flagIndex][3 * j] < 0) break;

    Vector3R vertices[3];
    for (int i = 0; i < 3; i++)
      vertices[i] = edgeVertex[triangleConnectionTable[flagIndex][3 * j + i]];

    surface->addElement(vertices);
  }
}


void MarchingCubes::marchTetrahedron(FieldFunction &func, Vector3R *position,
                                     bool *inside) {
  Vector3R edgeVertex[6];

  // Find which vertices are inside of the surface and which are outside
  int flagIndex = 0;
  for (int i = 0; i < 4; i++)
    if (!inside[i]) flagIndex |= 1 << i;

  // Find which edges are intersected by the surface
  int edgeFlags = tetrahedronEdgeFlags[flagIndex];

  // If the tetrahedron is entirely inside or outside of the surface, then
  // there will be no intersections
  if (!edgeFlags) return;

  // Find the point of intersection of the surface with each edge
  //  Then find the normal to the surface at those points
  for (int i = 0; i < 6; i++) {
    // if there is an intersection on this edge
    if (edgeFlags & (1 << i)) {
      int vert0 = tetrahedronEdgeConnection[i][0];
      int vert1 = tetrahedronEdgeConnection[i][1];

      edgeVertex[i] = func.getEdgeIntersect(position[vert0], inside[vert0],
                                            position[vert1], inside[vert1]);
    }
  }

  // Draw the triangles that were found. There can be up to 2 per tetrahedron
  for (int j = 0; j < 2; j++) {
    if (tetrahedronTriangles[flagIndex][3 * j] < 0) break;

    Vector3R vertices[3];
    for (int i = 0; i < 3; i++)
      vertices[i] = edgeVertex[tetrahedronTriangles[flagIndex][3 * j + i]];

    surface->addElement(vertices);
  }
}


void MarchingCubes::marchTetrahedrons(FieldFunction &func, const Vector3R &p,
                                      const Vector3R &scale) {
  //  tetrahedronsInACube lists the index of verticies from a cube
  //  that made up each of the six tetrahedrons within the cube
  static const int tetrahedronsInACube[6][4] = {
    {0, 5, 1, 6}, {0, 1, 2, 6}, {0, 2, 3, 6},
    {0, 3, 7, 6}, {0, 7, 4, 6}, {0, 4, 5, 6},
  };

  // Make a local copy of the cube's corner positions & locations
  Vector3R cubePosition[8];
  bool cubeInside[8];
  for (int i = 0; i < 8; i++) {
    cubePosition[i] = p + vertexOffset[i] * scale;
    cubeInside[i] = func.contains(cubePosition[i]);
  }

  Vector3R tetrahedronPosition[4];
  bool tetrahedronInside[4];
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 4; j++) {
      int v = tetrahedronsInACube[i][j];
      tetrahedronPosition[j] = cubePosition[v];
      tetrahedronInside[j] = cubeInside[v];
    }

    marchTetrahedron(func, tetrahedronPosition, tetrahedronInside);
  }
}


const int tetrahedronEdgeFlags[16] = {
  0x00, 0x0d, 0x13, 0x1e, 0x26, 0x2b, 0x35, 0x38,
  0x38, 0x35, 0x2b, 0x26, 0x1e, 0x13, 0x0d, 0x00,
};


//  For each of the possible vertex states listed in tetrahedronEdgeFlags
// there is a specific triangulation of the edge intersection points.
// tetrahedronTriangles lists all of them in the form of
//  0-2 edge triples with the list terminated by the invalid value -1.
//
//  I generated this table by hand
const int tetrahedronTriangles[16][7] = {
  {-1, -1, -1, -1, -1, -1, -1},
  {0,  3,  2,  -1, -1, -1, -1},
  {0,  1,  4,  -1, -1, -1, -1},
  {1,  4,  2,  2,  4,  3,  -1},

  {1,  2,  5,  -1, -1, -1, -1},
  {0,  3,  5,  0,  5,  1,  -1},
  {0,  2,  5,  0,  5,  4,  -1},
  {5,  4,  3,  -1, -1, -1, -1},

  {3,  4,  5,  -1, -1, -1, -1},
  {4,  5,  0,  5,  2,  0,  -1},
  {1,  5,  0,  5,  3,  0,  -1},
  {5,  2,  1,  -1, -1, -1, -1},

  {3,  4,  2,  2,  4,  1,  -1},
  {4,  1,  0,  -1, -1, -1, -1},
  {2,  3,  0,  -1, -1, -1, -1},
  {-1, -1, -1, -1, -1, -1, -1},
};

// tetrahedronEdgeConnection lists the index of the endpoint vertices for
// each of the 6 edges of the tetrahedron
const int tetrahedronEdgeConnection[6][2] = {
  {0, 1}, {1, 2}, {2, 0}, {0, 3}, {1, 3}, {2, 3}
};


//  For any edge, if one vertex is inside of the surface and the other is
// outside of the surface then the edge intersects the surface
//  For each of the 8 vertices of the cube can be two possible states : either
// inside or outside of the surface
//  For any cube the are 2^8=256 possible sets of vertex states
//  This table lists the edges intersected by the surface for all 256 possible
//  vertex states
//  There are 12 edges. For each entry in the table, if edge #n is intersected,
// then bit #n is set to 1
const int cubeEdgeFlags[256] = {
  0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f,
  0xb06, 0xc0a, 0xd03, 0xe09, 0xf00, 0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f,
  0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 0x230,
  0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936,
  0xe3a, 0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5,
  0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0, 0x460, 0x569,
  0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a,
  0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc,
  0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 0x650, 0x759, 0x453,
  0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53,
  0x859, 0x950, 0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc,
  0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca,
  0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9,
  0x7c0, 0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055,
  0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6,
  0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
  0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f,
  0x066, 0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af,
  0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0, 0xd30,
  0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636,
  0x13a, 0x033, 0x339, 0x230, 0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895,
  0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190, 0xf00, 0xe09,
  0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a,
  0x203, 0x109, 0x000,
};


//  For each of the possible vertex states listed in cubeEdgeFlags there is a
// specific triangulation of the edge intersection points.
// triangleConnectionTable lists all of them in the form of 0-5 edge triples
// with the list terminated by the invalid value -1.
//  For example: triangleConnectionTable[3] list the 2 triangles formed when
// corner[0] and corner[1] are inside of the surface, but the rest of the cube
// is not.
// 
//  I found this table in an example program someone wrote long ago. It was
//  probably generated by hand
const int triangleConnectionTable[256][16] = {
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  9,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  8,  3,  9,  8,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  1,  2,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  2,  10, 0,  2,  9,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {2,  8,  3,  2,  10, 8,  10, 9,  8,  -1, -1, -1, -1, -1, -1, -1},
  {3,  11, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  11, 2,  8,  11, 0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  9,  0,  2,  3,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  11, 2,  1,  9,  11, 9,  8,  11, -1, -1, -1, -1, -1, -1, -1},
  {3,  10, 1,  11, 10, 3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  10, 1,  0,  8,  10, 8,  11, 10, -1, -1, -1, -1, -1, -1, -1},
  {3,  9,  0,  3,  11, 9,  11, 10, 9,  -1, -1, -1, -1, -1, -1, -1},
  {9,  8,  10, 10, 8,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  7,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  3,  0,  7,  3,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  9,  8,  4,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  1,  9,  4,  7,  1,  7,  3,  1,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, 8,  4,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  4,  7,  3,  0,  4,  1,  2,  10, -1, -1, -1, -1, -1, -1, -1},
  {9,  2,  10, 9,  0,  2,  8,  4,  7,  -1, -1, -1, -1, -1, -1, -1},
  {2,  10, 9,  2,  9,  7,  2,  7,  3,  7,  9,  4,  -1, -1, -1, -1},
  {8,  4,  7,  3,  11, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {11, 4,  7,  11, 2,  4,  2,  0,  4,  -1, -1, -1, -1, -1, -1, -1},
  {9,  0,  1,  8,  4,  7,  2,  3,  11, -1, -1, -1, -1, -1, -1, -1},
  {4,  7,  11, 9,  4,  11, 9,  11, 2,  9,  2,  1,  -1, -1, -1, -1},
  {3,  10, 1,  3,  11, 10, 7,  8,  4,  -1, -1, -1, -1, -1, -1, -1},
  {1,  11, 10, 1,  4,  11, 1,  0,  4,  7,  11, 4,  -1, -1, -1, -1},
  {4,  7,  8,  9,  0,  11, 9,  11, 10, 11, 0,  3,  -1, -1, -1, -1},
  {4,  7,  11, 4,  11, 9,  9,  11, 10, -1, -1, -1, -1, -1, -1, -1},
  {9,  5,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  5,  4,  0,  8,  3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  5,  4,  1,  5,  0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {8,  5,  4,  8,  3,  5,  3,  1,  5,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, 9,  5,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  0,  8,  1,  2,  10, 4,  9,  5,  -1, -1, -1, -1, -1, -1, -1},
  {5,  2,  10, 5,  4,  2,  4,  0,  2,  -1, -1, -1, -1, -1, -1, -1},
  {2,  10, 5,  3,  2,  5,  3,  5,  4,  3,  4,  8,  -1, -1, -1, -1},
  {9,  5,  4,  2,  3,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  11, 2,  0,  8,  11, 4,  9,  5,  -1, -1, -1, -1, -1, -1, -1},
  {0,  5,  4,  0,  1,  5,  2,  3,  11, -1, -1, -1, -1, -1, -1, -1},
  {2,  1,  5,  2,  5,  8,  2,  8,  11, 4,  8,  5,  -1, -1, -1, -1},
  {10, 3,  11, 10, 1,  3,  9,  5,  4,  -1, -1, -1, -1, -1, -1, -1},
  {4,  9,  5,  0,  8,  1,  8,  10, 1,  8,  11, 10, -1, -1, -1, -1},
  {5,  4,  0,  5,  0,  11, 5,  11, 10, 11, 0,  3,  -1, -1, -1, -1},
  {5,  4,  8,  5,  8,  10, 10, 8,  11, -1, -1, -1, -1, -1, -1, -1},
  {9,  7,  8,  5,  7,  9,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  3,  0,  9,  5,  3,  5,  7,  3,  -1, -1, -1, -1, -1, -1, -1},
  {0,  7,  8,  0,  1,  7,  1,  5,  7,  -1, -1, -1, -1, -1, -1, -1},
  {1,  5,  3,  3,  5,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  7,  8,  9,  5,  7,  10, 1,  2,  -1, -1, -1, -1, -1, -1, -1},
  {10, 1,  2,  9,  5,  0,  5,  3,  0,  5,  7,  3,  -1, -1, -1, -1},
  {8,  0,  2,  8,  2,  5,  8,  5,  7,  10, 5,  2,  -1, -1, -1, -1},
  {2,  10, 5,  2,  5,  3,  3,  5,  7,  -1, -1, -1, -1, -1, -1, -1},
  {7,  9,  5,  7,  8,  9,  3,  11, 2,  -1, -1, -1, -1, -1, -1, -1},
  {9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7,  11, -1, -1, -1, -1},
  {2,  3,  11, 0,  1,  8,  1,  7,  8,  1,  5,  7,  -1, -1, -1, -1},
  {11, 2,  1,  11, 1,  7,  7,  1,  5,  -1, -1, -1, -1, -1, -1, -1},
  {9,  5,  8,  8,  5,  7,  10, 1,  3,  10, 3,  11, -1, -1, -1, -1},
  {5,  7,  0,  5,  0,  9,  7,  11, 0,  1,  0,  10, 11, 10, 0,  -1},
  {11, 10, 0,  11, 0,  3,  10, 5,  0,  8,  0,  7,  5,  7,  0,  -1},
  {11, 10, 5,  7,  11, 5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {10, 6,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  5,  10, 6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  0,  1,  5,  10, 6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  8,  3,  1,  9,  8,  5,  10, 6,  -1, -1, -1, -1, -1, -1, -1},
  {1,  6,  5,  2,  6,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  6,  5,  1,  2,  6,  3,  0,  8,  -1, -1, -1, -1, -1, -1, -1},
  {9,  6,  5,  9,  0,  6,  0,  2,  6,  -1, -1, -1, -1, -1, -1, -1},
  {5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8,  -1, -1, -1, -1},
  {2,  3,  11, 10, 6,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {11, 0,  8,  11, 2,  0,  10, 6,  5,  -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  9,  2,  3,  11, 5,  10, 6,  -1, -1, -1, -1, -1, -1, -1},
  {5,  10, 6,  1,  9,  2,  9,  11, 2,  9,  8,  11, -1, -1, -1, -1},
  {6,  3,  11, 6,  5,  3,  5,  1,  3,  -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  11, 0,  11, 5,  0,  5,  1,  5,  11, 6,  -1, -1, -1, -1},
  {3,  11, 6,  0,  3,  6,  0,  6,  5,  0,  5,  9,  -1, -1, -1, -1},
  {6,  5,  9,  6,  9,  11, 11, 9,  8,  -1, -1, -1, -1, -1, -1, -1},
  {5,  10, 6,  4,  7,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  3,  0,  4,  7,  3,  6,  5,  10, -1, -1, -1, -1, -1, -1, -1},
  {1,  9,  0,  5,  10, 6,  8,  4,  7,  -1, -1, -1, -1, -1, -1, -1},
  {10, 6,  5,  1,  9,  7,  1,  7,  3,  7,  9,  4,  -1, -1, -1, -1},
  {6,  1,  2,  6,  5,  1,  4,  7,  8,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  5,  5,  2,  6,  3,  0,  4,  3,  4,  7,  -1, -1, -1, -1},
  {8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,  6,  -1, -1, -1, -1},
  {7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,  9,  -1},
  {3,  11, 2,  7,  8,  4,  10, 6,  5,  -1, -1, -1, -1, -1, -1, -1},
  {5,  10, 6,  4,  7,  2,  4,  2,  0,  2,  7,  11, -1, -1, -1, -1},
  {0,  1,  9,  4,  7,  8,  2,  3,  11, 5,  10, 6,  -1, -1, -1, -1},
  {9,  2,  1,  9,  11, 2,  9,  4,  11, 7,  11, 4,  5,  10, 6,  -1},
  {8,  4,  7,  3,  11, 5,  3,  5,  1,  5,  11, 6,  -1, -1, -1, -1},
  {5,  1,  11, 5,  11, 6,  1,  0,  11, 7,  11, 4,  0,  4,  11, -1},
  {0,  5,  9,  0,  6,  5,  0,  3,  6,  11, 6,  3,  8,  4,  7,  -1},
  {6,  5,  9,  6,  9,  11, 4,  7,  9,  7,  11, 9,  -1, -1, -1, -1},
  {10, 4,  9,  6,  4,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  10, 6,  4,  9,  10, 0,  8,  3,  -1, -1, -1, -1, -1, -1, -1},
  {10, 0,  1,  10, 6,  0,  6,  4,  0,  -1, -1, -1, -1, -1, -1, -1},
  {8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1,  10, -1, -1, -1, -1},
  {1,  4,  9,  1,  2,  4,  2,  6,  4,  -1, -1, -1, -1, -1, -1, -1},
  {3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4,  -1, -1, -1, -1},
  {0,  2,  4,  4,  2,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {8,  3,  2,  8,  2,  4,  4,  2,  6,  -1, -1, -1, -1, -1, -1, -1},
  {10, 4,  9,  10, 6,  4,  11, 2,  3,  -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  2,  2,  8,  11, 4,  9,  10, 4,  10, 6,  -1, -1, -1, -1},
  {3,  11, 2,  0,  1,  6,  0,  6,  4,  6,  1,  10, -1, -1, -1, -1},
  {6,  4,  1,  6,  1,  10, 4,  8,  1,  2,  1,  11, 8,  11, 1,  -1},
  {9,  6,  4,  9,  3,  6,  9,  1,  3,  11, 6,  3,  -1, -1, -1, -1},
  {8,  11, 1,  8,  1,  0,  11, 6,  1,  9,  1,  4,  6,  4,  1,  -1},
  {3,  11, 6,  3,  6,  0,  0,  6,  4,  -1, -1, -1, -1, -1, -1, -1},
  {6,  4,  8,  11, 6,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {7,  10, 6,  7,  8,  10, 8,  9,  10, -1, -1, -1, -1, -1, -1, -1},
  {0,  7,  3,  0,  10, 7,  0,  9,  10, 6,  7,  10, -1, -1, -1, -1},
  {10, 6,  7,  1,  10, 7,  1,  7,  8,  1,  8,  0,  -1, -1, -1, -1},
  {10, 6,  7,  10, 7,  1,  1,  7,  3,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7,  -1, -1, -1, -1},
  {2,  6,  9,  2,  9,  1,  6,  7,  9,  0,  9,  3,  7,  3,  9,  -1},
  {7,  8,  0,  7,  0,  6,  6,  0,  2,  -1, -1, -1, -1, -1, -1, -1},
  {7,  3,  2,  6,  7,  2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {2,  3,  11, 10, 6,  8,  10, 8,  9,  8,  6,  7,  -1, -1, -1, -1},
  {2,  0,  7,  2,  7,  11, 0,  9,  7,  6,  7,  10, 9,  10, 7,  -1},
  {1,  8,  0,  1,  7,  8,  1,  10, 7,  6,  7,  10, 2,  3,  11, -1},
  {11, 2,  1,  11, 1,  7,  10, 6,  1,  6,  7,  1,  -1, -1, -1, -1},
  {8,  9,  6,  8,  6,  7,  9,  1,  6,  11, 6,  3,  1,  3,  6,  -1},
  {0,  9,  1,  11, 6,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {7,  8,  0,  7,  0,  6,  3,  11, 0,  11, 6,  0,  -1, -1, -1, -1},
  {7,  11, 6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {7,  6,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  0,  8,  11, 7,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  9,  11, 7,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {8,  1,  9,  8,  3,  1,  11, 7,  6,  -1, -1, -1, -1, -1, -1, -1},
  {10, 1,  2,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, 3,  0,  8,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1},
  {2,  9,  0,  2,  10, 9,  6,  11, 7,  -1, -1, -1, -1, -1, -1, -1},
  {6,  11, 7,  2,  10, 3,  10, 8,  3,  10, 9,  8,  -1, -1, -1, -1},
  {7,  2,  3,  6,  2,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {7,  0,  8,  7,  6,  0,  6,  2,  0,  -1, -1, -1, -1, -1, -1, -1},
  {2,  7,  6,  2,  3,  7,  0,  1,  9,  -1, -1, -1, -1, -1, -1, -1},
  {1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6,  -1, -1, -1, -1},
  {10, 7,  6,  10, 1,  7,  1,  3,  7,  -1, -1, -1, -1, -1, -1, -1},
  {10, 7,  6,  1,  7,  10, 1,  8,  7,  1,  0,  8,  -1, -1, -1, -1},
  {0,  3,  7,  0,  7,  10, 0,  10, 9,  6,  10, 7,  -1, -1, -1, -1},
  {7,  6,  10, 7,  10, 8,  8,  10, 9,  -1, -1, -1, -1, -1, -1, -1},
  {6,  8,  4,  11, 8,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  6,  11, 3,  0,  6,  0,  4,  6,  -1, -1, -1, -1, -1, -1, -1},
  {8,  6,  11, 8,  4,  6,  9,  0,  1,  -1, -1, -1, -1, -1, -1, -1},
  {9,  4,  6,  9,  6,  3,  9,  3,  1,  11, 3,  6,  -1, -1, -1, -1},
  {6,  8,  4,  6,  11, 8,  2,  10, 1,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, 3,  0,  11, 0,  6,  11, 0,  4,  6,  -1, -1, -1, -1},
  {4,  11, 8,  4,  6,  11, 0,  2,  9,  2,  10, 9,  -1, -1, -1, -1},
  {10, 9,  3,  10, 3,  2,  9,  4,  3,  11, 3,  6,  4,  6,  3,  -1},
  {8,  2,  3,  8,  4,  2,  4,  6,  2,  -1, -1, -1, -1, -1, -1, -1},
  {0,  4,  2,  4,  6,  2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  9,  0,  2,  3,  4,  2,  4,  6,  4,  3,  8,  -1, -1, -1, -1},
  {1,  9,  4,  1,  4,  2,  2,  4,  6,  -1, -1, -1, -1, -1, -1, -1},
  {8,  1,  3,  8,  6,  1,  8,  4,  6,  6,  10, 1,  -1, -1, -1, -1},
  {10, 1,  0,  10, 0,  6,  6,  0,  4,  -1, -1, -1, -1, -1, -1, -1},
  {4,  6,  3,  4,  3,  8,  6,  10, 3,  0,  3,  9,  10, 9,  3,  -1},
  {10, 9,  4,  6,  10, 4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  9,  5,  7,  6,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  4,  9,  5,  11, 7,  6,  -1, -1, -1, -1, -1, -1, -1},
  {5,  0,  1,  5,  4,  0,  7,  6,  11, -1, -1, -1, -1, -1, -1, -1},
  {11, 7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5,  -1, -1, -1, -1},
  {9,  5,  4,  10, 1,  2,  7,  6,  11, -1, -1, -1, -1, -1, -1, -1},
  {6,  11, 7,  1,  2,  10, 0,  8,  3,  4,  9,  5,  -1, -1, -1, -1},
  {7,  6,  11, 5,  4,  10, 4,  2,  10, 4,  0,  2,  -1, -1, -1, -1},
  {3,  4,  8,  3,  5,  4,  3,  2,  5,  10, 5,  2,  11, 7,  6,  -1},
  {7,  2,  3,  7,  6,  2,  5,  4,  9,  -1, -1, -1, -1, -1, -1, -1},
  {9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7,  -1, -1, -1, -1},
  {3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0,  -1, -1, -1, -1},
  {6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8,  -1},
  {9,  5,  4,  10, 1,  6,  1,  7,  6,  1,  3,  7,  -1, -1, -1, -1},
  {1,  6,  10, 1,  7,  6,  1,  0,  7,  8,  7,  0,  9,  5,  4,  -1},
  {4,  0,  10, 4,  10, 5,  0,  3,  10, 6,  10, 7,  3,  7,  10, -1},
  {7,  6,  10, 7,  10, 8,  5,  4,  10, 4,  8,  10, -1, -1, -1, -1},
  {6,  9,  5,  6,  11, 9,  11, 8,  9,  -1, -1, -1, -1, -1, -1, -1},
  {3,  6,  11, 0,  6,  3,  0,  5,  6,  0,  9,  5,  -1, -1, -1, -1},
  {0,  11, 8,  0,  5,  11, 0,  1,  5,  5,  6,  11, -1, -1, -1, -1},
  {6,  11, 3,  6,  3,  5,  5,  3,  1,  -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  10, 9,  5,  11, 9,  11, 8,  11, 5,  6,  -1, -1, -1, -1},
  {0,  11, 3,  0,  6,  11, 0,  9,  6,  5,  6,  9,  1,  2,  10, -1},
  {11, 8,  5,  11, 5,  6,  8,  0,  5,  10, 5,  2,  0,  2,  5,  -1},
  {6,  11, 3,  6,  3,  5,  2,  10, 3,  10, 5,  3,  -1, -1, -1, -1},
  {5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2,  -1, -1, -1, -1},
  {9,  5,  6,  9,  6,  0,  0,  6,  2,  -1, -1, -1, -1, -1, -1, -1},
  {1,  5,  8,  1,  8,  0,  5,  6,  8,  3,  8,  2,  6,  2,  8,  -1},
  {1,  5,  6,  2,  1,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  3,  6,  1,  6,  10, 3,  8,  6,  5,  6,  9,  8,  9,  6,  -1},
  {10, 1,  0,  10, 0,  6,  9,  5,  0,  5,  6,  0,  -1, -1, -1, -1},
  {0,  3,  8,  5,  6,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {10, 5,  6,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {11, 5,  10, 7,  5,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {11, 5,  10, 11, 7,  5,  8,  3,  0,  -1, -1, -1, -1, -1, -1, -1},
  {5,  11, 7,  5,  10, 11, 1,  9,  0,  -1, -1, -1, -1, -1, -1, -1},
  {10, 7,  5,  10, 11, 7,  9,  8,  1,  8,  3,  1,  -1, -1, -1, -1},
  {11, 1,  2,  11, 7,  1,  7,  5,  1,  -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  1,  2,  7,  1,  7,  5,  7,  2,  11, -1, -1, -1, -1},
  {9,  7,  5,  9,  2,  7,  9,  0,  2,  2,  11, 7,  -1, -1, -1, -1},
  {7,  5,  2,  7,  2,  11, 5,  9,  2,  3,  2,  8,  9,  8,  2,  -1},
  {2,  5,  10, 2,  3,  5,  3,  7,  5,  -1, -1, -1, -1, -1, -1, -1},
  {8,  2,  0,  8,  5,  2,  8,  7,  5,  10, 2,  5,  -1, -1, -1, -1},
  {9,  0,  1,  5,  10, 3,  5,  3,  7,  3,  10, 2,  -1, -1, -1, -1},
  {9,  8,  2,  9,  2,  1,  8,  7,  2,  10, 2,  5,  7,  5,  2,  -1},
  {1,  3,  5,  3,  7,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  7,  0,  7,  1,  1,  7,  5,  -1, -1, -1, -1, -1, -1, -1},
  {9,  0,  3,  9,  3,  5,  5,  3,  7,  -1, -1, -1, -1, -1, -1, -1},
  {9,  8,  7,  5,  9,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {5,  8,  4,  5,  10, 8,  10, 11, 8,  -1, -1, -1, -1, -1, -1, -1},
  {5,  0,  4,  5,  11, 0,  5,  10, 11, 11, 3,  0,  -1, -1, -1, -1},
  {0,  1,  9,  8,  4,  10, 8,  10, 11, 10, 4,  5,  -1, -1, -1, -1},
  {10, 11, 4,  10, 4,  5,  11, 3,  4,  9,  4,  1,  3,  1,  4,  -1},
  {2,  5,  1,  2,  8,  5,  2,  11, 8,  4,  5,  8,  -1, -1, -1, -1},
  {0,  4,  11, 0,  11, 3,  4,  5,  11, 2,  11, 1,  5,  1,  11, -1},
  {0,  2,  5,  0,  5,  9,  2,  11, 5,  4,  5,  8,  11, 8,  5,  -1},
  {9,  4,  5,  2,  11, 3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {2,  5,  10, 3,  5,  2,  3,  4,  5,  3,  8,  4,  -1, -1, -1, -1},
  {5,  10, 2,  5,  2,  4,  4,  2,  0,  -1, -1, -1, -1, -1, -1, -1},
  {3,  10, 2,  3,  5,  10, 3,  8,  5,  4,  5,  8,  0,  1,  9,  -1},
  {5,  10, 2,  5,  2,  4,  1,  9,  2,  9,  4,  2,  -1, -1, -1, -1},
  {8,  4,  5,  8,  5,  3,  3,  5,  1,  -1, -1, -1, -1, -1, -1, -1},
  {0,  4,  5,  1,  0,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {8,  4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5,  -1, -1, -1, -1},
  {9,  4,  5,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  11, 7,  4,  9,  11, 9,  10, 11, -1, -1, -1, -1, -1, -1, -1},
  {0,  8,  3,  4,  9,  7,  9,  11, 7,  9,  10, 11, -1, -1, -1, -1},
  {1,  10, 11, 1,  11, 4,  1,  4,  0,  7,  4,  11, -1, -1, -1, -1},
  {3,  1,  4,  3,  4,  8,  1,  10, 4,  7,  4,  11, 10, 11, 4,  -1},
  {4,  11, 7,  9,  11, 4,  9,  2,  11, 9,  1,  2,  -1, -1, -1, -1},
  {9,  7,  4,  9,  11, 7,  9,  1,  11, 2,  11, 1,  0,  8,  3,  -1},
  {11, 7,  4,  11, 4,  2,  2,  4,  0,  -1, -1, -1, -1, -1, -1, -1},
  {11, 7,  4,  11, 4,  2,  8,  3,  4,  3,  2,  4,  -1, -1, -1, -1},
  {2,  9,  10, 2,  7,  9,  2,  3,  7,  7,  4,  9,  -1, -1, -1, -1},
  {9,  10, 7,  9,  7,  4,  10, 2,  7,  8,  7,  0,  2,  0,  7,  -1},
  {3,  7,  10, 3,  10, 2,  7,  4,  10, 1,  10, 0,  4,  0,  10, -1},
  {1,  10, 2,  8,  7,  4,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  9,  1,  4,  1,  7,  7,  1,  3,  -1, -1, -1, -1, -1, -1, -1},
  {4,  9,  1,  4,  1,  7,  0,  8,  1,  8,  7,  1,  -1, -1, -1, -1},
  {4,  0,  3,  7,  4,  3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {4,  8,  7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9,  10, 8,  10, 11, 8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  0,  9,  3,  9,  11, 11, 9,  10, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  10, 0,  10, 8,  8,  10, 11, -1, -1, -1, -1, -1, -1, -1},
  {3,  1,  10, 11, 3,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  2,  11, 1,  11, 9,  9,  11, 8,  -1, -1, -1, -1, -1, -1, -1},
  {3,  0,  9,  3,  9,  11, 1,  2,  9,  2,  11, 9,  -1, -1, -1, -1},
  {0,  2,  11, 8,  0,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {3,  2,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {2,  3,  8,  2,  8,  10, 10, 8,  9,  -1, -1, -1, -1, -1, -1, -1},
  {9,  10, 2,  0,  9,  2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {2,  3,  8,  2,  8,  10, 0,  1,  8,  1,  10, 8,  -1, -1, -1, -1},
  {1,  10, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {1,  3,  8,  9,  1,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  9,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  3,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};
