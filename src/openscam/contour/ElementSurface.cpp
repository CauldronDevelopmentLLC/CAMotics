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

#include "ElementSurface.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

#include <openscam/view/GL.h>
#include <openscam/stl/STL.h>

#include <map>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


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

    vertices.clear(); normals.clear();
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
  if (normal.length() == 0) return; // Degenerate element, skip
  normal /= length; // Normalize

  for (unsigned i = 0; i < dim; i++) {
    for (unsigned j = 0; j < 3; j++) {
      this->vertices.push_back(vertices[i][j]);
      normals.push_back(normal[j]);
    }

    addNormalLine(vertices[i], normal);
  }
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


void ElementSurface::exportSTL(STL &stl) {
  SmartPointer<Vector3D>::Array p = new Vector3D[dim];

  for (unsigned i = 0; i < count; i++) {
    unsigned offset = i * 3 * dim;

    // Compute surface normal
    Vector3D normal;
    for (unsigned j = 0; j < dim; j++)
      normal += Vector3D(normals[offset + j + 0],
                         normals[offset + j + 1],
                         normals[offset + j + 2]);
    normal = -normal.normalize();

    for (unsigned j = 0; j < dim; j++)
      p[j] = Vector3D(vertices[offset + j * 3 + 0],
                      vertices[offset + j * 3 + 1],
                      vertices[offset + j * 3 + 2]);

    if (dim == 3) stl.push_back(Facet(p[0], p[1], p[2], normal));
    else THROW("STL quad export not yet supported");
  }
}


namespace {
  struct normals_t {
    Vector3D normal;
    unsigned count;
    normals_t() : count(0) {}
  };
}


void ElementSurface::smooth() {
  // NOTE Many incident vertices are missed due to small floating point
  // differences
  typedef map<Vector3R, normals_t> vertNorms_t;
  vertNorms_t vertNorms;
  vertNorms_t::iterator it;

  // Gather adjacent normals
  for (unsigned i = 0; i < count * dim; i++) {
    unsigned offset = i * 3;
    Vector3D v(vertices[offset], vertices[offset + 1], vertices[offset + 2]);
    Vector3D n(normals[offset], normals[offset + 1], normals[offset + 2]);

    it = vertNorms.insert(vertNorms_t::value_type(v, normals_t())).first;
    it->second.normal += n;
    it->second.count++;
  }

  // Average adjacent normals
  for (it = vertNorms.begin(); it != vertNorms.end(); it++)
    it->second.normal = (it->second.normal / it->second.count).normalize();

  // Apply averaged normals
  for (unsigned i = 0; i < count * dim; i++) {
    unsigned offset = i * 3;
    Vector3D v(vertices[offset], vertices[offset + 1], vertices[offset + 2]);

    it = vertNorms.find(v);
    if (it == vertNorms.end()) THROWS("Couldn't find vertex");
    normals[offset + 0] = it->second.normal.x();
    normals[offset + 1] = it->second.normal.y();
    normals[offset + 2] = it->second.normal.z();
  }
}
