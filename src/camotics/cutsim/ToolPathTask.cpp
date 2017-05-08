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

#include "ToolPathTask.h"

#include <camotics/TaskFilter.h>
#include <camotics/cutsim/Project.h>
#include <camotics/cutsim/Simulation.h>
#include <camotics/sim/Controller.h>
#include <camotics/gcode/Interpreter.h>
#include <camotics/machine/Machine.h>

#include <cbang/util/DefaultCatch.h>
#include <cbang/util/SmartFunctor.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/os/Subprocess.h>

#include <cbang/iostream/VectorDevice.h>
#include <cbang/iostream/TeeFilter.h>

#include <cbang/log/AsyncCopyStreamToLog.h>

#include <boost/ref.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathTask::ToolPathTask(const Project &project) :
  tools(project.getToolTable()),
  units(project.getUnits() == ToolUnits::UNITS_MM ? Units::METRIC :
        Units::IMPERIAL), simJSON(project.toString()), errors(0) {

  for (Project::iterator it = project.begin(); it != project.end(); it++)
    files.push_back((*it)->getAbsolutePath());
}


ToolPathTask::~ToolPathTask() {
  interrupt();
}


void ToolPathTask::run() {
  // Task tracking
  Task::begin();
  SmartFunctor<Task, double (Task::*)()> endTask(this, &Task::end);

  // Setup
  path = new ToolPath(tools);
  // TODO load machine configuration, including rapidFeed
  Machine machine(*path);
  machine.reset();
  Controller controller(machine, tools);

  // Interpret code
  for (unsigned i = 0; i < files.size() && !Task::shouldQuit(); i++) {
    string filename = files[i];

    if (!SystemUtilities::exists(filename)) continue;

    Task::update(0, "Running " + filename);

    SmartPointer<istream> stream;
    io::filtering_istream filter;

    // Track the file load
    TaskFilter taskFilter(*this);
    filter.push(boost::ref(taskFilter));

    // Copy the GCode
    gcode = new vector<char>;
    VectorStream<> vstream(*gcode);
    TeeFilter teeFilter(vstream);
    filter.push(teeFilter);

    // Generate tool path
    if (String::endsWith(filename, ".tpl")) {
      // Get executable name
      string cmd =
        SystemUtilities::joinPath
        (SystemUtilities::dirname(SystemUtilities::getExecutablePath()),
         "tplang");
#ifdef _WIN32
      cmd += ".exe";
#endif

      if (!SystemUtilities::exists(cmd)) cmd = "tplang";

      // Build args
      vector<string> args;
      args.push_back(cmd);

      // Add units
      args.push_back(string("--") + String::toLower(units.toString()));

      // Add simulation JSON
      args.push_back("--sim-json=" + simJSON);

      // Add file
      args.push_back(filename);

      // Create process
      proc = new Subprocess;

      // Add pipe
      unsigned pipe = proc->createPipe(false);
      args.push_back("--pipe");
      args.push_back(String((uint64_t)proc->getPipeHandle(pipe)));

      // Execute
      proc->exec(args, Subprocess::REDIR_STDOUT |
                 Subprocess::MERGE_STDOUT_AND_STDERR |
                 Subprocess::W32_HIDE_WINDOW,
                 ProcessPriority::PRIORITY_LOW);

      // Get pipe stream
      stream = proc->getStream(pipe);
      filter.push(*stream);

      // Copy output to log
      logCopier = new AsyncCopyStreamToLog(proc->getStream(1));
      logCopier->start();

      // Change file name so GCode error messages make sense
      filename = "<generated gcode>";

    } else // Assume it's just GCode
      filter.push(io::file_source(filename));

    try {
      InputSource src(filter, filename);

      // Parse GCode
      Interpreter interp(controller, SmartPointer<Task>::Phony(this));
      interp.read(src);
      errors += interp.getErrorCount();

    } catch (const Exception &e) {
      LOG_ERROR(e);
      errors++;
    }

    // Wait for Subprocess
    if (!proc.isNull() && proc->waitFor(5, 10)) errors++;

    // Stop the log copier
    if (!logCopier.isNull()) {
      logCopier->join();
      logCopier.release();
    }
  }

  proc.release();
}


void ToolPathTask::interrupt() {
  Task::interrupt();
  if (!proc.isNull()) try {proc->kill(true);} CATCH_ERROR;
  if (!logCopier.isNull()) logCopier->stop();
}
