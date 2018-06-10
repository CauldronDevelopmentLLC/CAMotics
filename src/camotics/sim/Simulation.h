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


#include "Workpiece.h"

#include <gcode/ToolTable.h>
#include <gcode/ToolPath.h>
#include <camotics/render/RenderMode.h>

#include <cbang/SmartPointer.h>
#include <cbang/json/Serializable.h>

#include <string>

namespace cb {namespace JSON {class Sink;}}


namespace CAMotics {
  class Workpiece;

  class Simulation : public cb::JSON::Serializable {
  public:
    cb::SmartPointer<GCode::ToolPath> path;
    Workpiece workpiece;
    double resolution;
    double time;
    RenderMode mode;
    unsigned threads;

    Simulation(const cb::SmartPointer<GCode::ToolPath> &path,
               const Workpiece &workpiece, double resolution, double time,
               RenderMode mode, unsigned threads) :
      path(path), workpiece(workpiece), resolution(resolution),
      time(time), mode(mode), threads(threads) {}

    const GCode::ToolTable &getTools() const {return path->getTools();}

    std::string computeHash() const;

    virtual void write(cb::JSON::Sink &sink, bool withPath) const;

    // From JSON::Serializable
    using cb::JSON::Serializable::read;
    using cb::JSON::Serializable::write;
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const {write(sink, false);}
  };
}
