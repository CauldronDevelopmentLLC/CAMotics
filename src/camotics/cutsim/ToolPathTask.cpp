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

#include <cbang/util/DefaultCatch.h>
#include <cbang/util/SmartFunctor.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/os/Subprocess.h>

#include <cbang/log/AsyncCopyStreamToLog.h>

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


ToolPathTask::~ToolPathTask() {
  interrupt();
}


void ToolPathTask::run() {
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

    SmartPointer<istream> stream;
    SmartPointer<TaskFilter> taskFilter;

    if (String::endsWith(files[i], ".tpl")) {
      // Get executable name
      string cmd =
        SystemUtilities::dirname(SystemUtilities::getExecutablePath()) +
        "/tplang";
#ifdef _WIN32
      cmd += ".exe";
#endif

      if (!SystemUtilities::exists(cmd)) cmd = "tplang";

      // Create process
      proc = new Subprocess;

      // Add file
      cmd += " " + files[i];

      // Add pipe
      unsigned pipe = proc->createPipe(false);
      cmd += SSTR(" --pipe 0x" << hex << proc->getPipeHandle(pipe));

      // Execute
      LOG_DEBUG(1, "Executing: " << cmd);
      proc->exec(cmd, Subprocess::SHELL | Subprocess::REDIR_STDOUT |
                 Subprocess::MERGE_STDOUT_AND_STDERR,
                 ProcessPriority::PRIORITY_LOW);

      // Get pipe stream
      stream = SmartPointer<istream>::Phony(&proc->getStream(pipe));

      // Copy output to log
      logCopier = new AsyncCopyStreamToLog(proc->getStream(1));
      logCopier->start();

    } else { // Assume it's just GCode
      // Track the file load
      io::filtering_istream *filter = new io::filtering_istream;
      stream = filter;

      taskFilter =
        new TaskFilter(*this, SystemUtilities::getFileSize(files[i]));
      filter->push(boost::ref(*taskFilter));
      filter->push(io::file(files[i], ios_base::in));
    }

    InputSource src(*stream, files[i]);

    // Parse GCode
    Interpreter interp(controller, SmartPointer<Task>::Phony(this));
    interp.read(src);
    errors += interp.getErrorCount();

    // Wait for Subprocess
    if (!proc.isNull() && proc->wait()) errors++;

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
  if (!proc.isNull()) proc->kill(true);
  if (!logCopier.isNull()) logCopier->stop();
}


void ToolPathTask::move(const Move &move) {
  Machine::move(move);
  path->add(move);
}
