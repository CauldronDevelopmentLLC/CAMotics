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

#include "TriangleMesh.h"

#include <camotics/Task.h>

#include <cbang/log/Logger.h>
#include <cbang/time/Timer.h>

#include <algorithm>
#include <map>

using namespace std;
using namespace cb;
using namespace CAMotics;


TriangleMesh::TriangleMesh(const TriangleMesh &o) :
  vertices(o.vertices), normals(o.normals) {}


void TriangleMesh::Vertex::set(const Vector3D &v) {
  for (unsigned i = 0; i < 3; i++) (*this)[i] = v[i];
}


Vector3D TriangleMesh::Triangle::computeNormal() const {
  if (!(vertices[0] && vertices[1] && vertices[2]))
    THROW("Triangle has null vertex");

  Vector3D u = *vertices[0] - *vertices[1];
  Vector3D v = *vertices[1] - *vertices[2];

  return u.cross(v).normalize();
}


void TriangleMesh::Triangle::updateNormal() {normal = computeNormal();}


bool TriangleMesh::Triangle::has(Vertex &v) const {
  for (unsigned i = 0; i < 3; i++)
    if (vertices[i] == &v) return true;

  return false;
}


void TriangleMesh::Triangle::replace(Vertex &a, Vertex &b) {
  for (unsigned i = 0; i < 3; i++)
    if (vertices[i] == &a) {
      vertices[i] = &b;
      return;
    }

  THROW("Vertex " << a << " not found in triangle");
}


bool TriangleMesh::Triangle::isFlipped() const {
  return computeNormal().dot(normal) < 0;
}


void TriangleMesh::Triangle::unflip() {
  if (isFlipped()) swap(vertices[0], vertices[1]);
}


bool TriangleMesh::Triangle::wouldFlip(Vertex &a, Vertex &b) const {
  Triangle t;
  Vertex mid((a + b) / 2.0);

  for (unsigned i = 0; i < 3; i++)
    t.vertices[i] =
      (*vertices[i] == a || *vertices[i] == b) ? &mid : vertices[i];

  t.updateNormal();

  return normal.dot(t.normal) < 0;
}


void TriangleMesh::Vertex::findNeighbors(VertexSet &neighbors) const {
  for (unsigned i = 0; i < triangles.size(); i++) {
    Triangle &t = *triangles[i];

    if (t.deleted) continue;

    for (unsigned j = 0; j < 3; j++)
      if (t.vertices[j] != this) neighbors.insert(t.vertices[j]);
  }
}


bool TriangleMesh::Vertex::coplaner(const Vector3D &normal,
                                    double tolerance) const {
  for (unsigned i = 0; i < triangles.size() - 1; i++) {
    if (triangles[i]->deleted) continue;

    const Vector3D &na = triangles[i]->normal;
    const Vector3D &nb = triangles[i + 1]->normal;

    double cosAngle = na.dot(nb);
    if (cosAngle < 1 - tolerance) {
      cosAngle = na.cross(nb).normalize().dot(normal);
      if (-1 + tolerance < cosAngle && cosAngle < 1 - tolerance) return false;
    }
  }

  return true;
}


bool TriangleMesh::Vertex::wouldFlip(Vertex &o) {
  for (unsigned i = 0; i < triangles.size(); i++)
    if (!triangles[i]->deleted && triangles[i]->wouldFlip(*this, o))
      return true;

  return false;
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
}


void TriangleMesh::weld(Task &task, float threshold) {
  if (vertices.empty()) return;

  task.begin("Welding triangle mesh");

  unsigned count = vertices.size() / 3;

  vector<unsigned> vertexIndex(count);
  for (unsigned i = 0; i < count; i++) vertexIndex[i] = i;

  for (unsigned axis = 0; axis < 3; axis++) {
    if (!task.update((double)axis / 3)) return;
    sort(vertexIndex.begin(), vertexIndex.end(), AxisSort(vertices, axis));

    for (unsigned i = 0; i < count - 1; i++) {
      unsigned a = vertexIndex[i] * 3 + axis;
      unsigned b = vertexIndex[i + 1] * 3 + axis;

      if (vertices[b] < vertices[a] + threshold) vertices[b] = vertices[a];
    }
  }
}


void TriangleMesh::reduce(Task &task) {
  unsigned count = getTriangleCount();

  // Build triangles and find unique vertices
  vector<SmartPointer<Vertex> > vertices;
  vector<Triangle> triangles(count);

  typedef map<Vector3D, unsigned> unique_vertices_t;
  unique_vertices_t uniqueVertices;
  Vector3D v;
  unsigned index = 0;

  task.begin("Reducing mesh: finding vertices");
  for (unsigned i = 0; i < count; i++) {
    if (!task.update((double)i / count)) return;

    Triangle &t = triangles[i];

    for (unsigned j = 0; j < 3; j++) {
      for (unsigned k = 0; k < 3; k++) v[k] = this->vertices[index++];

      unsigned pos = uniqueVertices.insert
        (unique_vertices_t::value_type(v, vertices.size())).first->second;

      if (pos == vertices.size()) vertices.push_back(new Vertex(v));

      t.vertices[j] = vertices[pos].get();
      vertices[pos]->triangles.push_back(&t);
    }

    t.updateNormal();
  }

  // Collapse edges
  task.begin("Reducing mesh: collapsing edges");
  for (unsigned i = 0; i < vertices.size(); i++) {
    if (!task.update((double)i / vertices.size())) return;

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
      Vector3D n = (v - neighbor).normalize();
      if (!v.coplaner(n)) continue;
      if (!neighbor.coplaner(n)) continue;

      // Make sure the neighbor does not have more then two neighbors in common
      VertexSet neighborNeighbors;
      neighbor.findNeighbors(neighborNeighbors);
      if (moreThan2InCommon(neighbors, neighborNeighbors)) continue;

      // Check for flips
      if (v.wouldFlip(neighbor) || neighbor.wouldFlip(v)) continue;

      // Choose neighbor with the least number of neighbors
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
    merge->set((v + *merge) / 2.0);

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

  // Flip edges
  task.begin("Reducing mesh: flipping edges");
  for (unsigned i = 0; i < vertices.size(); i++) {
    if (!task.update((double)i / vertices.size())) return;

    Vertex &v = *vertices[i];

    // Find neighbors
    VertexSet neighbors;
    v.findNeighbors(neighbors);

    VertexSet::iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); it++) {
      Vertex &neighbor = **it;

      // Find neighbor's neighbors
      VertexSet neighborNeighbors;
      neighbor.findNeighbors(neighborNeighbors);

      // Find common vertices
      VertexSet common;
      VertexSet::iterator it2;
      for (it2 = neighborNeighbors.begin(); it2 != neighborNeighbors.end();
           it2++)
        if (*it != &v) common.insert(*it);

      // Ignore if more then two common vertices
      if (common.size() != 2) continue;

      // Evaluate flip
      it2 = common.begin();
      Vertex &a = **it2++;
      Vertex &b = **it2;

      if (v.distanceSquared(neighbor) && a.distanceSquared(b)) continue;

      // Find triangles
      vector<Triangle *> triangles;
      for (unsigned j = 0; j < v.triangles.size(); j++)
        if (v.triangles[j]->has(neighbor)) triangles.push_back(v.triangles[j]);

      if (triangles.size() != 2)
        THROW("Wrong number of matching triangles " << triangles.size());

      // Flip edge
      for (unsigned j = 0; j < triangles.size(); j++) {
        if (triangles[j]->has(a)) triangles[j]->replace(v, b);
        else if (triangles[j]->has(b)) triangles[j]->replace(neighbor, a);

        triangles[j]->unflip();
      }
    }
  }

  // Reconstruct
  this->vertices.clear();
  this->normals.clear();

  task.begin("Reducing mesh: reconstructing");
  for (unsigned i = 0; i < triangles.size(); i++) {
    if (!task.update((double)i / triangles.size())) return;

    Triangle &t = triangles[i];
    if (t.deleted) continue;
    t.updateNormal();
    if (!t.normal.isReal()) continue; // Degenerate, discard

    for (unsigned j = 0; j < 3; j++) {
      Vertex &v = *t.vertices[j];

      for (unsigned k = 0; k < 3; k++) {
        this->vertices.push_back(v[k]);
        this->normals.push_back(t.normal[k]);
      }
    }
  }
}


bool TriangleMesh::moreThan2InCommon(VertexSet &vs1, VertexSet &vs2) {
  unsigned count = 0;
  VertexSet::iterator it1 = vs1.begin();
  VertexSet::iterator it2 = vs2.end();

  while (it1 != vs1.end() && it2 != vs2.end()) {
    if (**it1 == **it2) {
      if (++count == 3) return true;
      it1++;
      it2++;

    } else if (**it1 < **it2) it1++;
    else it2++;
  }

  return false;
}
