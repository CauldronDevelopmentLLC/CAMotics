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

#ifndef CAMOTICS_SURFACE_H
#define CAMOTICS_SURFACE_H

#include <camotics/Geom.h>

#include <cbang/io/OutputSink.h>


namespace CAMotics {
  class STLSink;
  class Task;

  class Surface {
  public:
    virtual ~Surface() {}

    virtual cb::SmartPointer<Surface> copy() const = 0;
    virtual uint64_t getCount() const = 0;
    virtual Rectangle3R getBounds() const = 0;
    virtual void draw() = 0;
    virtual void write(STLSink &sink, Task *task = 0) const = 0;
    virtual void reduce(Task &task) = 0;

    void writeSTL(const cb::OutputSink &sink, bool binary,
                  const std::string &name, const std::string &hash) const;
  };
}

#endif // CAMOTICS_SURFACE_H
