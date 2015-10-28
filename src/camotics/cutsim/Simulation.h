/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#ifndef CAMOTICS_SIMULATION_H
#define CAMOTICS_SIMULATION_H

#include "Workpiece.h"

#include <camotics/sim/ToolTable.h>
#include <camotics/render/RenderMode.h>

#include <cbang/SmartPointer.h>

#include <string>

namespace cb {namespace JSON {class Sink;}}


namespace CAMotics {
  class ToolPath;
  class Workpiece;

  class Simulation {
  public:
    ToolTable tools;
    cb::SmartPointer<ToolPath> path;
    Workpiece workpiece;
    double resolution;
    double time;
    RenderMode mode;

    Simulation(const ToolTable &tools, const cb::SmartPointer<ToolPath> &path,
               const Workpiece &workpiece, double resolution, double time,
               RenderMode mode) :
      tools(tools), path(path), workpiece(workpiece), resolution(resolution),
      time(time), mode(mode) {}

    std::string computeHash() const;
    void write(cb::JSON::Sink &sink) const;
  };
}

#endif // CAMOTICS_SIMULATION_H

