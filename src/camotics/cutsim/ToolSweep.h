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

#ifndef CAMOTICS_TOOL_SWEEP_H
#define CAMOTICS_TOOL_SWEEP_H

#include "AABBTree.h"
#include "ToolPath.h"

#include <camotics/contour/FieldFunction.h>

#include <cbang/StdTypes.h>
#include <cbang/SmartPointer.h>

#include <vector>
#include <limits>


namespace CAMotics {
  class ToolTable;
  class Sweep;

  class ToolSweep : public FieldFunction, public AABBTree {
    cb::SmartPointer<ToolPath> path;
    std::vector<cb::SmartPointer<Sweep> > sweeps;
    real time;

  public:
    ToolSweep(const cb::SmartPointer<ToolPath> &path,
              real time = std::numeric_limits<real>::max());

    real getTime() const {return time;}
    void setTime(real time) {this->time = time;}

    void getChangeBounds(std::vector<Rectangle3R> &bboxes, real startTime,
                         real endTime) const;

    // From FieldFunction
    bool contains(const Vector3R &p);
  };
}

#endif // CAMOTICS_TOOL_SWEEP_H
