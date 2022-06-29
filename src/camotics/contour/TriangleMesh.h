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

#pragma once

#include <cbang/geom/Vector.h>

#include <vector>
#include <set>
#include <limits>


namespace CAMotics {
  class Task;

  class TriangleMesh {
    struct Triangle;
    struct Vertex;
    struct VertexSort;
    typedef std::set<Vertex *, VertexSort> VertexSet;


    struct Vertex : cb::Vector3D {
      std::vector<Triangle *> triangles;
      bool deleted;

      Vertex(const cb::Vector3D &v) : cb::Vector3D(v), deleted(false) {}

      void set(const cb::Vector3D &v);
      void findNeighbors(VertexSet &neighbors) const;
      bool coplaner(const cb::Vector3D &normal,
                    double tolerance = 0.0001) const;
      bool wouldFlip(Vertex &o);
    };


    struct VertexSort {
      bool operator() (const Vertex *a, const Vertex *b) const {return *a < *b;}
    };


    struct Triangle {
      Vertex *vertices[3];
      cb::Vector3D normal;
      bool deleted;

      Triangle() : deleted(false) {vertices[0] = vertices[1] = vertices[2] = 0;}


      cb::Vector3D computeNormal() const;
      void updateNormal();

      bool has(Vertex &v) const;
      void replace(Vertex &a, Vertex &b);

      bool isFlipped() const;
      void unflip();
      bool wouldFlip(Vertex &a, Vertex &b) const;
    };

  protected:
    std::vector<float> vertices;
    std::vector<float> normals;

  public:
    TriangleMesh() {}
    TriangleMesh(const TriangleMesh &o);

    unsigned getTriangleCount() const {return vertices.size() / 9;}

    void weld(Task &task,
              float threshold = std::numeric_limits<float>::epsilon() * 10);
    void reduce(Task &task);

  protected:
    static bool moreThan2InCommon(VertexSet &vs1, VertexSet &vs2);
  };
}
