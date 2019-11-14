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

#include "TriangleSurface.h"

#include "TriangleMesh.h"
#include "GridTree.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

#include <camotics/Task.h>
#ifdef CAMOTICS_GUI
#include <camotics/view/GL.h>
#endif

#include <stl/Source.h>
#include <stl/Sink.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


TriangleSurface::TriangleSurface(const GridTree &tree) : finalized(false) {
  vbufs[0] = 0;
  add(tree);
}


TriangleSurface::TriangleSurface(STL::Source &source, Task *task) :
  finalized(false) {
  vbufs[0] = 0;
  read(source, task);
}


TriangleSurface::TriangleSurface(vector<SmartPointer<Surface> > &surfaces) :
  finalized(false) {
  vbufs[0] = 0;

  for (unsigned i = 0; i < surfaces.size(); i++) {
    TriangleSurface *s = dynamic_cast<TriangleSurface *>(surfaces[i].get());
    if (!s) THROW("Expected an TriangleSurface");

    // Copy surface data
    vertices.insert(vertices.end(), s->vertices.begin(), s->vertices.end());
    normals.insert(normals.end(), s->normals.begin(), s->normals.end());
    bounds.add(s->bounds);

    surfaces[i] = 0; // Free memory as we go
  }
}


TriangleSurface::TriangleSurface(const TriangleSurface &o) :
  TriangleMesh(o), finalized(false), bounds(o.bounds) {
  vbufs[0] = 0;
}


TriangleSurface::TriangleSurface() : finalized(false) {
  vbufs[0] = 0;
}


TriangleSurface::~TriangleSurface() {
#ifdef CAMOTICS_GUI
  try {
    if (vbufs[0]) getGLFuncs().glDeleteBuffers(2, vbufs);
  } catch (...) {}
#endif
}


void TriangleSurface::finalize() {
  if (finalized) return;

#ifdef CAMOTICS_GUI
  GLFuncs &glFuncs = getGLFuncs();

  if (!vbufs[0]) glFuncs.glGenBuffers(2, vbufs);

  // Vertices
  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, vbufs[0]);
  glFuncs.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                       &vertices[0], GL_STATIC_DRAW);

  // Normals
  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, vbufs[1]);
  glFuncs.glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
                       &normals[0], GL_STATIC_DRAW);

  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif // CAMOTICS_GUI

  finalized = true;
}


void TriangleSurface::add(const Vector3F vertices[3]) {
  // Compute face normal
  Vector3F normal =
    (vertices[1] - vertices[0]).cross(vertices[2] - vertices[0]);

  // Normalize
  double length = normal.length();
  if (length == 0) return; // Degenerate element, skip
  normal /= length;

  // Add it
  add(vertices, normal);
}


void TriangleSurface::add(const Vector3F vertices[3], const Vector3F &normal) {
  for (unsigned i = 0; i < 3; i++) {
    bounds.add(vertices[i]);

    for (unsigned j = 0; j < 3; j++) {
      this->vertices.push_back(vertices[i][j]);
      normals.push_back(normal[j]);
    }
  }
}


void TriangleSurface::add(const GridTree &tree) {
  unsigned start = getCount();

  tree.gather(vertices, normals);

  for (unsigned i = start; i < vertices.size(); i += 3)
    bounds.add(Vector3F(vertices[i], vertices[i + 1], vertices[i + 2]));
}


SmartPointer<Surface> TriangleSurface::copy() const {
  return new TriangleSurface(*this);
}


#ifdef CAMOTICS_GUI
void TriangleSurface::draw() {
  if (!getCount()) return; // Nothing to draw

  finalize();

#if 0 // TODO GL
  GLFuncs &glFuncs = getGLFuncs();

  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, vbufs[0]);
  glFuncs.glVertexPointer(3, GL_FLOAT, 0, 0);

  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, vbufs[1]);
  glFuncs.glNormalPointer(GL_FLOAT, 0, 0);

  glFuncs.glBindBuffer(GL_ARRAY_BUFFER, 0);

  glFuncs.glEnableClientState(GL_VERTEX_ARRAY);
  glFuncs.glEnableClientState(GL_NORMAL_ARRAY);

  glFuncs.glDrawArrays(GL_TRIANGLES, 0, getCount() * 3);

  glFuncs.glDisableClientState(GL_NORMAL_ARRAY);
  glFuncs.glDisableClientState(GL_VERTEX_ARRAY);
#endif
}
#endif // CAMOTICS_GUI


void TriangleSurface::clear() {
  finalized = false;

  vertices.clear();
  normals.clear();

  bounds = Rectangle3D();
}


void TriangleSurface::read(STL::Source &source, Task *task) {
  clear();

  uint32_t facets = source.getFacetCount(); // ASCII STL files return 0

  Vector3F v[3];
  Vector3F n;

  if (task) task->begin("Reading STL surface");

  for (unsigned i = 0; source.hasMore() && (!task || !task->shouldQuit());
       i++) {
    // Read facet
    source.readFacet(v[0], v[1], v[2], n);

    // Validate
    bool valid = n.isReal();
    for (unsigned j = 0; j < 3; j++)
      if (!v[j].isReal()) valid = false;

    if (!valid) {
      LOG_ERROR("Invalid facet in STL: normal=" << n << " triangle=("
                << v[0] << ", " << v[1] << ", " << v[2] << ")");
      continue;
    }

    add(v, n);

    if (task) {
      if (!facets && 100000 < i) i = 0;
      if (!task->update((double)i / (facets ? facets : 100000))) return;
    }
  }
}


void TriangleSurface::write(STL::Sink &sink, Task *task) const {
  Vector3F p[3];

  if (task) task->begin("Writing STL surface");

  for (unsigned i = 0; i < getCount() && (!task || !task->shouldQuit()); i++) {
    unsigned offset = i * 9;

    // In an STL file, there's only one normal per facet.
    // They are the same anyway.
    Vector3F normal =
      Vector3F(normals[offset + 0], normals[offset + 1], normals[offset + 2]);

    for (unsigned j = 0; j < 3; j++)
      for (unsigned k = 0; k < 3; k++)
        p[j][k] = vertices[offset + j * 3 + k];

    sink.writeFacet(p[0], p[1], p[2], normal);

    if (task) task->update((double)i / getCount());
  }
}


void TriangleSurface::reduce(Task &task) {
  weld(task);
  TriangleMesh::reduce(task);
}


void TriangleSurface::read(const JSON::Value &value) {
  // TODO
}


void TriangleSurface::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insertList("vertices");
  for (unsigned i = 0; i < vertices.size(); i++)
    sink.append(vertices[i]);
  sink.endList();

  // Only output every third normal since they are the same for a triangle.
  sink.insertList("normals");
  for (unsigned i = 0; i < normals.size() / 9; i++)
    for (unsigned j = 0; j < 3; j++)
      sink.append(normals[i * 9 + j]);
  sink.endList();

  sink.endDict();
}
