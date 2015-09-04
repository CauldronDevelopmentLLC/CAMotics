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

#include "ToolPathTask.h"

#include <camotics/cutsim/Project.h>
#include <camotics/sim/Controller.h>
#include <camotics/gcode/Interpreter.h>

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <cbang/js/Javascript.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathTask::ToolPathTask(const Project &project) :
  tools(project.getToolTable()), errors(0) {

  for (Project::iterator it = project.begin(); it != project.end(); it++)
    files.push_back((*it)->getAbsolutePath());
}


void ToolPathTask::run() {
  v8::Locker locker;

  // Task tracking
  Task::begin();
  SmartFunctor<Task, double (Task::*)()> endTask(this, &Task::end);

  // Setup
  Machine::reset();
  Controller controller(*this, tools);
  path = new ToolPath(tools);

  // Interpret code
  for (unsigned i = 0; i < files.size() && !Task::shouldQuit(); i++) {
    if (!SystemUtilities::exists(files[i])) continue;

    Task::update(0, "Running " + files[i]);

    if (String::endsWith(files[i], ".tpl")) {
      tplang::TPLContext ctx(cout, *this, tools);
      ctx.pushPath(files[i]);

      try {
        tplang::Interpreter(ctx).read(files[i]);
      } catch (const Exception &e) {
        LOG_ERROR(e);
        errors++;
      }

    } else {
      // Assume GCode
      Interpreter interp(controller, SmartPointer<Task>::Phony(this));
      interp.read(files[i]);
      errors += interp.getErrorCount();
    }
  }
}


void ToolPathTask::interrupt() {
  js::Javascript::terminate(); // End TPL code
  Task::interrupt();
}


void ToolPathTask::move(const Move &move) {
  Machine::move(move);
  path->add(move);
}
