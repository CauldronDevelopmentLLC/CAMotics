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

#ifndef CAMOTICS_CUT_SIM_H
#define CAMOTICS_CUT_SIM_H

#include "Simulation.h"

#include <camotics/Task.h>
#include <camotics/Geom.h>
#include <camotics/sim/Machine.h>

#include <cbang/SmartPointer.h>

#include <vector>
#include <string>


namespace cb {class Options;}

namespace CAMotics {
  class ToolTable;
  class ToolPath;
  class Surface;
  class Project;

  class CutSim : public Machine, public Task {
    unsigned threads;
    cb::SmartPointer<ToolPath> path;

  public:
    CutSim(cb::Options &options);
    ~CutSim();

    cb::SmartPointer<ToolPath>
    computeToolPath(const cb::SmartPointer<ToolTable> &tools,
                    const std::vector<std::string> &files);
    cb::SmartPointer<ToolPath> computeToolPath(const Project &project);

    cb::SmartPointer<Surface> computeSurface(const Simulation &sim);

    void reduceSurface(Surface &surface);

    // From Task
    void interrupt();

    // From Machine
    void move(const Move &move);
  };
}

#endif // CAMOTICS_CUT_SIM_H

