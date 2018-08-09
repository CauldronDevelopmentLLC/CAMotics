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


#include "Surface.h"
#include "TriangleMesh.h"


#include <cbang/SmartPointer.h>

#include <vector>


namespace STL {class Source;}

namespace CAMotics {
  class GridTree;

  class TriangleSurface : public Surface, public TriangleMesh {
    bool finalized;

    unsigned vbufs[2];
    bool useVBOs;

    cb::Rectangle3D bounds;

  public:
    TriangleSurface(const GridTree &tree);
    TriangleSurface(STL::Source &source, Task *task = 0);
    TriangleSurface(std::vector<cb::SmartPointer<Surface> > &surfaces);
    TriangleSurface(const TriangleSurface &o);
    TriangleSurface();
    virtual ~TriangleSurface();

    void finalize(bool withVBOs);
    void add(const cb::Vector3F vertices[3]);
    void add(const cb::Vector3F vertices[3], const cb::Vector3F &normal);
    void add(const GridTree &tree);

    // From Surface
    cb::SmartPointer<Surface> copy() const;
    uint64_t getCount() const {return TriangleMesh::getCount();}
    cb::Rectangle3D getBounds() const {return bounds;}
#ifdef CAMOTICS_GUI
    void draw(bool withVBOs);
#endif
    void clear();
    void read(STL::Source &source, Task *task = 0);
    void write(STL::Sink &sink, Task *task = 0) const;
    void reduce(Task &task);

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
