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

#include "HermiteSlice.h"

using namespace cb;
using namespace OpenSCAM;

// Positions, relative to vertex 0, of each of the 8 vertices of a cube
static const int vertexOffset[8][3] = {
  {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
  {1, 1, 1}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0},
};


void HermiteSlice::compute() {
  const SampleSlice *slices[2] = {&*this->first, &*this->second};
  FieldFunction &func = slices[0]->getFunction();
  const Vector2R &start = slices[0]->getBounds().getMin();
  real step = slices[0]->getStep();
  Vector3R vertex(start.x(), start.y(), slices[0]->getZ());

  // Allocate space
  unsigned count = (dims.x() + 1) * (dims.y() + 1);
  signs.reserve(count);
  for (unsigned i = 0; i < 3; i++) edgeIndices[i].reserve(count);
  edges.push_back(Edge()); // Skip edge index 0

  for (unsigned y = 0; y <= dims.y(); y++) {
    vertex.y() = start.y() + step * y;

    for (unsigned x = 0; x <= dims.x(); x++) {
      vertex.x() = start.x() + step * x;

      // Gather samples & signs
      real samples[8];
      bool signs[8];
      uint8_t cellSigns = 0;
      for (unsigned i = 0; i < 8; i++) {
        const int *offset = vertexOffset[i];
        samples[i] = (*slices[offset[2]])[y + offset[1]][x + offset[0]];
        signs[i] = samples[i] <= 0;
        if (signs[i]) cellSigns |= 1 << i;
      }

      // Store cell signs
      this->signs.push_back(cellSigns);

      // Compute 3 edges
      for (unsigned i = 0; i < 3; i++) {
        if (signs[0] != signs[i + 1]) {
          // Other vertex
          Vector3R vertex2 = vertex;
          vertex[i] += step;

          // Allocate edge
          edgeIndices[i].push_back(edges.size());
          edges.push_back(Edge());
          Edge &edge = edges.back();

          // Compute edge
          edge.vertex =
            func.getEdgeIntersect(vertex, samples[0], vertex2, samples[i + 1]);
          // TODO func.getNormal() was removed
          //edge.normal = func.getNormal(edge.vertex);

        } else edgeIndices[i].push_back(0);
      }
    }
  }

  first = second = 0; // Free sample slices
}
