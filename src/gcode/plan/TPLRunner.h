/******************************************************************************\

             CAMotics is an Open-Source simulation and CAM software.
     Copyright (C) 2011-2021 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "Runner.h"

#include <gcode/machine/MachineAdapter.h>

#include <cbang/config.h>
#include <cbang/os/Thread.h>
#include <cbang/os/Condition.h>
#include <cbang/io/InputSource.h>

namespace tplang {class TPLContext;}


namespace GCode {
  class MachinePipeline;
  class PlannerConfig;

  class TPLRunner :
    public Runner, public MachineAdapter, protected cb::Thread,
    protected cb::Condition {
    cb::InputSource source;
    cb::SmartPointer<tplang::TPLContext> ctx;
    bool done = false;

  public:
    TPLRunner(MachinePipeline &pipeline, const cb::InputSource &source,
              const PlannerConfig &config);
    ~TPLRunner();

    // From MachineAdapter
    void enter() const;
    void exit() const;

    // From Runner
    bool hasMore() const;
    void step();

    // From cb::Thread
    void run();
  };
}
