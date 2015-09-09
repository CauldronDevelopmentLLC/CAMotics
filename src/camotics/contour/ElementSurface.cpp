/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#include "ElementSurface.h"

#include "TriangleMesh.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

#include <camotics/Task.h>
#include <camotics/view/GL.h>
#include <camotics/stl/STLSource.h>
#include <camotics/stl/STLSink.h>

#include <map>
#include <limits>
#include <algorithm>

using namespace std;
using namespace cb;
using namespace CAMotics;


ElementSurface::ElementSurface(STLSource &source, Task *task) :
  dim(3), finalized(false), count(0) {
  vbufs[0] = 0;
  read(source, task);
}


ElementSurface::ElementSurface(vector<SmartPointer<Surface> > &surfaces) :
  dim(0), finalized(false), count(0) {
  vbufs[0] = 0;

  for (unsigned i = 0; i < surfaces.size(); i++) {
    ElementSurface *s = dynamic_cast<ElementSurface *>(surfaces[i].get());
    if (!s) THROW("Expected an ElementSurface");

    if (!i) dim = s->dim;
    else if (dim != s->dim) THROWS("Surface dimension mismatch");

    // Copy surface data
    vertices.insert(vertices.end(), s->vertices.begin(), s->vertices.end());
    normals.insert(normals.end(), s->normals.begin(), s->normals.end());
    count += s->count;
    bounds.add(s->bounds);

    surfaces[i] = 0; // Free memory as we go
  }
}


ElementSurface::ElementSurface(const ElementSurface &o) :
  dim(o.dim), finalized(false), vertices(o.vertices), normals(o.normals),
  count(o.count), bounds(o.bounds) {
  vbufs[0] = 0;
}


ElementSurface::ElementSurface(unsigned dim) :
  dim(dim), finalized(false), count(0) {
  vbufs[0] = 0;
  if (dim < 2 || 3 < dim) THROWS("Invalid dimension");
}


ElementSurface::~ElementSurface() {
  if (glDeleteBuffers && vbufs[0]) glDeleteBuffers(2, vbufs);
}


void ElementSurface::finalize() {
  if (finalized) return;

  if (glGenBuffers) {
    glGenBuffers(2, vbufs);

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, vbufs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                    &vertices[0], GL_STATIC_DRAW);

    // Normals
    glBindBuffer(GL_ARRAY_BUFFER, vbufs[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
                    &normals[0], GL_STATIC_DRAW);
  }

  finalized = true;
}


void ElementSurface::addElement(const Vector3R *vertices) {
  count++;

  // Add to bounds
  for (unsigned i = 0; i < dim; i++) bounds.add(vertices[i]);

  // Compute face normal
  Vector3R normal =
    (vertices[1] - vertices[0]).cross(vertices[dim - 1] - vertices[0]);
  real length = normal.length();
  if (length == 0) return; // Degenerate element, skip
  normal /= length; // Normalize

  for (unsigned i = 0; i < dim; i++)
    for (unsigned j = 0; j < 3; j++) {
      this->vertices.push_back(vertices[i][j]);
      normals.push_back(normal[j]);
    }
}


SmartPointer<Surface> ElementSurface::copy() const {
  return new ElementSurface(*this);
}


void ElementSurface::draw() {
  if (!count) return; // Nothing to draw

  finalize();

  if (glBindBuffer) {
    glBindBuffer(GL_ARRAY_BUFFER, vbufs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbufs[1]);
    glNormalPointer(GL_FLOAT, 0, 0);

  } else {
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    glNormalPointer(GL_FLOAT, 0, &normals[0]);
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  int mode;
  switch (dim) {
  case 3: mode = GL_TRIANGLES; break;
  case 4: mode = GL_QUADS; break;
  default: THROWS("Invalid surface element dimension " << dim);
  }

  glDrawArrays(mode, 0, count * dim);

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // Unbind  buffer
  if (glBindBuffer) glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void ElementSurface::clear() {
  finalized = false;

  if (glDeleteBuffers && vbufs[0]) glDeleteBuffers(2, vbufs);
  vertices.clear();
  normals.clear();

  count = 0;
  bounds = Rectangle3R();
}


void ElementSurface::read(STLSource &source, Task *task) {
  if (dim != 3) THROW("STL quad import not supported");

  clear();

  uint32_t facets = source.getFacetCount(); // ASCII STL files return 0
  if (facets) {
    normals.reserve(9 * facets);
    vertices.reserve(9 * facets);
  }

  Vector3F v[3];
  Vector3F n;

  for (unsigned i = 0; source.hasMore(); i++) {
    // Read facet
    source.readFacet(v[0], v[1], v[2], n);

    // Validate
    bool valid = n.isReal();
    for (unsigned j = 0; j < 3; j++)
      if (!v[j].isReal()) valid = false;

    if (!valid) {
      LOG_ERROR("Invalid vector in STL");
      continue;
    }

    // Normals
    for (unsigned j = 0; j < 3; j++)
      for (unsigned k = 0; k < 3; k++)
        normals.push_back(n[k]); // STL has only a face normal

    // Vertices
    for (unsigned j = 0; j < 3; j++) {
      for (unsigned k = 0; k < 3; k++)
        vertices.push_back(v[j][k]);

      bounds.add(v[j]);
    }

    if (task) {
      if (!facets && 100000 < i) i = 0;
      task->update((double)i / (facets ? facets : 100000),
                   "Reading STL surface");
    }

    count++;
  }

  task->update(1, "Idle");
}


void ElementSurface::write(STLSink &sink, Task *task) const {
  if (dim != 3) THROW("STL quad export not supported");

  Vector3R p[3];

  for (unsigned i = 0; i < count; i++) {
    unsigned offset = i * 9;

    // Compute surface normal
    Vector3R normal;
    for (unsigned j = 0; j < 3; j++)
      normal += Vector3R(normals[offset + j * 3 + 0],
                         normals[offset + j * 3 + 1],
                         normals[offset + j * 3 + 2]);
    normal = normal.normalize();

    for (unsigned j = 0; j < 3; j++)
      for (unsigned k = 0; k < 3; k++)
        p[j][k] = vertices[offset + j * 3 + k];

    sink.writeFacet(p[0], p[1], p[2], normal);

    if (task) task->update((double)i / count, "Writing STL surface");
  }
}


void ElementSurface::reduce(Task &task) {
  if (dim != 3) {
    LOG_WARNING(__func__ << "() only implemented for triangles");
    return;
  }

  TriangleMesh mesh(vertices, normals);

  mesh.weld();
  count = mesh.reduce(task);
}
