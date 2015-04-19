/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

#include <openscam/view/GL.h>
#include <openscam/stl/STL.h>

#include <map>
#include <limits>
#include <algorithm>

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
  struct AxisSort {
    const vector<float> &vertices;
    int axis;

    AxisSort(const vector<float> &vertices, int axis) :
      vertices(vertices), axis(axis) {}

    bool operator() (unsigned i, unsigned j) {
      return vertices[i * 3 + axis] < vertices[j * 3 + axis];
    }
  };


  struct Triangle;
  struct Vertex;
  struct VertexSort;
  typedef set<Vertex *, VertexSort> VertexSet;


  struct Vertex {
    Vector3R v;
    vector<Triangle *> triangles;
    bool deleted;

    Vertex(const Vector3R &v) : v(v), deleted(false) {}

    void findNeighbors(VertexSet &neighbors) const;
    bool coplaner(const Vector3R &normal, double tolerance = 0.0001) const;
    bool wouldFlip(Vertex &o);
    double weight(Vertex &o);
  };


  struct VertexSort {
    bool operator() (const Vertex *a, const Vertex *b) {return a->v < b->v;}
  };


  struct Triangle {
    Vertex *vertices[3];
    Vector3R normal;
    bool deleted;

    Triangle() : deleted(false) {vertices[0] = vertices[1] = vertices[2] = 0;}


    double areaSquared() const {
      double a = vertices[0]->v.length();
      double b = vertices[1]->v.length();
      double c = vertices[2]->v.length();
      double s = (a + b + c) / 2;

      return s * (s - a) * (s - b) * (s - c);
    }


    double weight() const {
      double a = vertices[0]->v.length();
      double b = vertices[1]->v.length();
      double c = vertices[2]->v.length();

      return (a + b + c) / min(a, min(b, c));
    }


    void computeNormal() {
      if (!(vertices[0] && vertices[1] && vertices[2]))
        THROW("Triangle has null vertex");

      Vector3R u = vertices[0]->v - vertices[1]->v;
      Vector3R v = vertices[1]->v - vertices[2]->v;

      normal = u.cross(v).normalize();
    }


    bool wouldFlip(Vertex &a, Vertex &b) const {
      Triangle t;
      Vertex mid((a.v + b.v) / 2.0);

      for (unsigned i = 0; i < 3; i++)
        t.vertices[i] =
          (vertices[i]->v == a.v || vertices[i]->v == b.v) ? &mid : vertices[i];

      t.computeNormal();

      return normal.dot(t.normal) < 0;
    }
  };


  void Vertex::findNeighbors(VertexSet &neighbors) const {
    for (unsigned i = 0; i < triangles.size(); i++) {
      Triangle &t = *triangles[i];

      if (t.deleted) continue;

      for (unsigned j = 0; j < 3; j++)
        if (t.vertices[j] != this) neighbors.insert(t.vertices[j]);
    }
  }


  bool Vertex::coplaner(const Vector3R &normal, double tolerance) const {
    for (unsigned i = 0; i < triangles.size() - 1; i++) {
      if (triangles[i]->deleted) continue;

      const Vector3R &na = triangles[i]->normal;
      const Vector3R &nb = triangles[i + 1]->normal;

      double cosAngle = na.dot(nb);
      if (cosAngle < 1 - tolerance) {
        cosAngle = na.cross(nb).normalize().dot(normal);
        if (cosAngle < 1 - tolerance) return false;
      }
    }

    return true;
  }


  bool Vertex::wouldFlip(Vertex &o) {
    for (unsigned i = 0; i < triangles.size(); i++)
      if (!triangles[i]->deleted && triangles[i]->wouldFlip(*this, o))
        return true;

    return false;
  }


  double Vertex::weight(Vertex &o) {
    return 0;
  }


  bool moreThan2InCommon(VertexSet &vs1, VertexSet &vs2) {
    unsigned count = 0;
    VertexSet::iterator it1 = vs1.begin();
    VertexSet::iterator it2 = vs2.end();

    while (it1 != vs1.end() && it2 != vs2.end()) {
      if ((*it1)->v == (*it2)->v) {
        if (++count == 3) return true;
        it1++;
        it2++;

      } else if ((*it1)->v < (*it2)->v) it1++;
      else it2++;
    }

    return false;
  }
}


void ElementSurface::simplify() {
  if (dim != 3) {
    LOG_WARNING(__func__ << "() only implemented for triangles");
    return;
  }

  // Weld vertices
  float delta = numeric_limits<float>::epsilon() * 10;
  vector<unsigned> vertexIndex(count * dim);
  for (unsigned i = 0; i < count * dim; i++) vertexIndex[i] = i;

  for (unsigned axis = 0; axis < 3; axis++) {
    sort(vertexIndex.begin(), vertexIndex.end(), AxisSort(vertices, axis));

    for (unsigned i = 0; i < count * 3 - 1; i++) {
      unsigned a = vertexIndex[i] * 3 + axis;
      unsigned b = vertexIndex[i + 1] * 3 + axis;

      if (vertices[b] < vertices[a] + delta) vertices[b] = vertices[a];
    }
  }

  // Build triangles and find unique vertices
  vector<SmartPointer<Vertex> > vertices;
  vector<Triangle> triangles(count);

  typedef map<Vector3R, unsigned> unique_vertices_t;
  unique_vertices_t uniqueVertices;
  Vector3R v;
  unsigned index = 0;

  for (unsigned i = 0; i < count; i++) {
    Triangle &t = triangles[i];

    for (unsigned j = 0; j < dim; j++) {
      for (unsigned k = 0; k < 3; k++) v[k] = this->vertices[index++];

      unsigned pos = uniqueVertices.insert
        (unique_vertices_t::value_type(v, vertices.size())).first->second;

      if (pos == vertices.size()) vertices.push_back(new Vertex(v));

      t.vertices[j] = vertices[pos].get();
      vertices[pos]->triangles.push_back(&t);
    }

    t.computeNormal();
  }

  // Collapse edges
  for (unsigned i = 0; i < vertices.size(); i++) {
    Vertex &v = *vertices[i];

    // Find neighbors
    VertexSet neighbors;
    v.findNeighbors(neighbors);

    // Choose vertex to merge with
    unsigned minNeighbors = numeric_limits<unsigned>::max();
    Vertex *merge = 0;

    VertexSet::iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); it++) {
      Vertex &neighbor = **it;

      // Check if adjacent triangles are coplaner
      Vector3R n = (v.v - neighbor.v).normalize();
      if (!v.coplaner(n)) continue;
      if (!neighbor.coplaner(n)) continue;

      // Make sure the neighbor does not have more then two neighbors in common
      VertexSet neighborNeighbors;
      neighbor.findNeighbors(neighborNeighbors);
      if (moreThan2InCommon(neighbors, neighborNeighbors)) continue;

      // Check for flips
      if (v.wouldFlip(neighbor) || neighbor.wouldFlip(v)) continue;

      unsigned numNeighbors = neighborNeighbors.size();
      if (numNeighbors < minNeighbors) {
        merge = &neighbor;
        minNeighbors = numNeighbors;
      }
    }

    if (!merge) continue;

    // Delete this vertex
    v.deleted = true;

    // Update triangles
    merge->v = (v.v + merge->v) / 2.0;

    for (unsigned j = 0; j < v.triangles.size(); j++) {
      Triangle &t = *v.triangles[j];
      if (t.deleted) continue;

      // Delete triangle
      for (unsigned k = 0; k < 3; k++)
        if (t.vertices[k] == merge) t.deleted = true;

      if (t.deleted) continue;

      // Merge triangle
      for (unsigned k = 0; k < 3; k++)
        if (t.vertices[k] == &v) t.vertices[k] = merge;

      // Add triangle
      bool added = false;
      for (unsigned k = 0; k < merge->triangles.size(); k++)
        if (merge->triangles[k]->deleted) {
          merge->triangles[k] = &t;
          added = true;
          break;
        }

      if (!added) merge->triangles.push_back(&t);
    }
  }

  // Reconstruct
  this->vertices.clear();
  normals.clear();
  count = 0;

  for (unsigned i = 0; i < triangles.size(); i++) {
    Triangle &t = triangles[i];
    if (t.deleted) continue;

    count++;

    for (unsigned j = 0; j < dim; j++) {
      Vertex &v = *t.vertices[j];

      for (unsigned k = 0; k < 3; k++) {
        this->vertices.push_back(v.v[k]);
        this->normals.push_back(t.normal[k]);
      }
    }
  }
}
