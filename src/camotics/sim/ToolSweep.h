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

#include "AABBTree.h"
#include <gcode/ToolPath.h>

#include <camotics/contour/FieldFunction.h>

#include <cbang/StdTypes.h>
#include <cbang/SmartPointer.h>

#include <vector>
#include <limits>


namespace GCode {class ToolTable;}

namespace CAMotics {
  class Sweep;

  class ToolSweep : public FieldFunction, public AABBTree {
    cb::SmartPointer<GCode::ToolPath> path;
    std::vector<cb::SmartPointer<Sweep> > sweeps;

    double startTime;
    double endTime;

    cb::SmartPointer<MoveLookup> change;

  public:
    ToolSweep(const cb::SmartPointer<GCode::ToolPath> &path,
              double startTime = 0,
              double endTime = std::numeric_limits<double>::max());

    void setStartTime(double startTime) {this->startTime = startTime;}
    void setEndTime(double endTime) {this->endTime = endTime;}

    const cb::SmartPointer<MoveLookup> &getChange() const {return change;}
    void setChange(const cb::SmartPointer<MoveLookup> &change)
    {this->change = change;}

    // From FieldFunction
    bool cull(const cb::Rectangle3D &r) const;
    double depth(const cb::Vector3D &p) const;

    static cb::SmartPointer<Sweep> getSweep(const GCode::Tool &tool);
  };
}
