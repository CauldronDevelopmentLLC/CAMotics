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

#include <cbang/geom/Rectangle.h>
#include <cbang/io/OutputSink.h>
#include <cbang/json/Serializable.h>

#include <functional>
#include <vector>


namespace STL {class Sink;}

namespace CAMotics {
  class Task;

  class Surface : cb::JSON::Serializable {
  public:
    virtual ~Surface() {}

    virtual cb::SmartPointer<Surface> copy() const = 0;
    virtual uint64_t getTriangleCount() const = 0;
    virtual cb::Rectangle3D getBounds() const = 0;
    typedef std::function<void (const std::vector<float> &vertices,
                                const std::vector<float> &normals)> vert_cb_t;
    virtual void getVertices(vert_cb_t cb) const = 0;
    virtual void write(STL::Sink &sink, Task *task = 0) const = 0;
    virtual void reduce(Task &task) = 0;

    void writeSTL(const cb::OutputSink &sink, bool binary,
                  const std::string &name, const std::string &hash) const;

    // From cb::JSON::Serializable
    using cb::JSON::Serializable::read;
    using cb::JSON::Serializable::write;
  };
}
