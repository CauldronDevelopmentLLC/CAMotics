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

#ifndef CAMOTICS_TOOL_PATH_TASK_H
#define CAMOTICS_TOOL_PATH_TASK_H

#include <camotics/Task.h>
#include <camotics/cutsim/ToolPath.h>
#include <camotics/sim/ToolTable.h>
#include <camotics/machine/Machine.h>

#include <string>
#include <vector>


namespace cb {
  class Subprocess;
  class Thread;
}

namespace CAMotics {
  class ToolTable;
  class ToolPath;
  class Project;

  class ToolPathTask : public Machine, public Task {
    ToolTable tools;
    std::vector<std::string> files;

    unsigned errors;
    cb::SmartPointer<ToolPath> path;

    cb::SmartPointer<cb::Subprocess> proc;
    cb::SmartPointer<cb::Thread> logCopier;

    public:
    ToolPathTask(const Project &project);
    ~ToolPathTask();

    unsigned getErrorCount() const {return errors;}
    const cb::SmartPointer<ToolPath> &getPath() const {return path;}

    // From Task
    void run();
    void interrupt();

    // From Machine
    void move(const Move &move);
  };
}

#endif // CAMOTICS_TOOL_PATH_TASK_H

