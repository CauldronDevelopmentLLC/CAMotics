/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_CUT_SIM_H
#define OPENSCAM_CUT_SIM_H

#include <openscam/sim/Machine.h>
#include <openscam/sim/Controller.h>


namespace cb {class Options;}

namespace OpenSCAM {
  class ToolTable;
  class ToolPath;
  class CutWorkpiece;
  class Project;

  class CutSim : public Machine {
    cb::SmartPointer<ToolTable> tools;
    cb::SmartPointer<ToolPath> path;
    cb::SmartPointer<CutWorkpiece> cutWP;

    Controller controller;

  public:
    CutSim(cb::Options &options);
    ~CutSim();

    cb::SmartPointer<ToolTable> getToolTable() {return tools;}
    cb::SmartPointer<ToolPath> getToolPath() const {return path;}
    cb::SmartPointer<CutWorkpiece> getCutWorkpiece() const {return cutWP;}
    Controller &getController() {return controller;}
    const Controller &getController() const {return controller;}

    void init(Project &project);
    void reset();

    // From Machine
    void move(const Move &move);
  };
}

#endif // OPENSCAM_CUT_SIM_H

