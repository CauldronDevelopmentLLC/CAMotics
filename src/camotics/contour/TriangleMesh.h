/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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


#include <camotics/Geom.h>

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


    struct Vertex : Vector3R {
      std::vector<Triangle *> triangles;
      bool deleted;

      Vertex(const Vector3R &v) : Vector3R(v), deleted(false) {}

      void set(const Vector3R &v);
      void findNeighbors(VertexSet &neighbors) const;
      bool coplaner(const Vector3R &normal, double tolerance = 0.0001) const;
      bool wouldFlip(Vertex &o);
    };


    struct VertexSort {
      bool operator() (const Vertex *a, const Vertex *b) {return *a < *b;}
    };


    struct Triangle {
      Vertex *vertices[3];
      Vector3R normal;
      bool deleted;

      Triangle() : deleted(false) {vertices[0] = vertices[1] = vertices[2] = 0;}


      Vector3R computeNormal() const;
      void updateNormal();

      bool has(Vertex &v) const;
      void replace(Vertex &a, Vertex &b);

      bool isFlipped() const;
      void unflip();
      bool wouldFlip(Vertex &a, Vertex &b) const;
    };

    double lastUpdate;

  protected:
    std::vector<float> vertices;
    std::vector<float> normals;

  public:
    TriangleMesh() : lastUpdate(0) {}
    TriangleMesh(const TriangleMesh &o);

    unsigned getCount() const {return vertices.size() / 9;}

    void weld(float threshold = std::numeric_limits<float>::epsilon() * 10);
    void reduce(Task &task);

  protected:
    bool update(Task &task, unsigned step, unsigned steps, unsigned current,
                unsigned total);

    static bool moreThan2InCommon(VertexSet &vs1, VertexSet &vs2);
  };
}
