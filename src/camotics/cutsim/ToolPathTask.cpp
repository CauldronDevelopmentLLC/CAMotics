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

#include "ToolPathTask.h"

#include <camotics/TaskFilter.h>
#include <camotics/cutsim/Project.h>
#include <camotics/sim/Controller.h>
#include <camotics/gcode/Interpreter.h>

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <cbang/js/Javascript.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/os/SystemUtilities.h>

#include <boost/ref.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathTask::ToolPathTask(const Project &project) :
  tools(project.getToolTable()), errors(0) {

  for (Project::iterator it = project.begin(); it != project.end(); it++)
    files.push_back((*it)->getAbsolutePath());
}


void ToolPathTask::run() {
  js::Isolate::ScopePtr scope = isolate.getScope();

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
      // Track the file load
      TaskFilter taskFilter(*this, SystemUtilities::getFileSize(files[i]));
      io::filtering_istream stream;
      stream.push(boost::ref(taskFilter));
      stream.push(io::file(files[i], ios_base::in));
      InputSource src(stream, files[i]);

      // Assume GCode
      Interpreter interp(controller, SmartPointer<Task>::Phony(this));
      interp.read(src);
      errors += interp.getErrorCount();
    }
  }
}


void ToolPathTask::interrupt() {
  isolate.interrupt();
  Task::interrupt();
}


void ToolPathTask::move(const Move &move) {
  Machine::move(move);
  path->add(move);
}
