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

#include "ToolPathTask.h"

#include <camotics/TaskFilter.h>
#include <camotics/project/Project.h>
#include <camotics/sim/Simulation.h>

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <gcode/interp/Interpreter.h>

#include <gcode/machine/MachineState.h>
#include <gcode/machine/MachineLinearizer.h>
#include <gcode/machine/MachineUnitAdapter.h>
#include <gcode/machine/GCodeMachine.h>
#include <gcode/machine/MoveSink.h>

#include <cbang/Catch.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/os/Subprocess.h>

#include <cbang/iostream/VectorDevice.h>
#include <cbang/iostream/TeeFilter.h>

#include <cbang/log/AsyncCopyStreamToLog.h>
#include <cbang/io/StringInputSource.h>

#include <cbang/js/JSInterrupted.h>

#include <boost/ref.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;

#include <sstream>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathTask::ToolPathTask(const Project::Project &project) :
  tools(project.getTools()), units(project.getUnits()),
  simJSON(project.toString()), controller(machine, tools),
  path(new GCode::ToolPath(tools)) {

  for (unsigned i = 0; i < project.getFileCount(); i++)
    files.push_back(project.getFile(i)->getPath());

  // Save GCode stream
  SmartPointer<ostream>::Phony gcodePtr(&gcode);

  // Create machine pipeline
  // TODO load machine configuration, including rapidFeed & maxArcError
  machine.add(new GCode::MachineUnitAdapter);
  machine.add(new GCode::MachineLinearizer);
  machine.add(new GCode::MoveSink(*path));
  if (units != GCode::Units::METRIC)
    machine.add(new GCode::MachineUnitAdapter(GCode::Units::METRIC, units));
  machine.add(new GCode::GCodeMachine(gcodePtr, units));
  machine.add(new GCode::MachineState);
}


ToolPathTask::~ToolPathTask() {interrupt();}


void ToolPathTask::interpGCode(const InputSource &source) {
  GCode::Interpreter interp(controller);
  interp.push(source);
  machine.start();

  try {
    while (!Task::shouldQuit() && interp.hasMore() && errors < 32)
      try {
        interp(interp.next());

      } catch (const Exception &e) {
        LOG_ERROR(e);
        errors++;
      }
  } catch (const GCode::EndProgram &) {}
  machine.end();
}


void ToolPathTask::runTPL(const string &filename) {
  Task::begin("Running TPL");

  // Get executable name
  string xdir = SystemUtilities::dirname(SystemUtilities::getExecutablePath());
  string cmd = SystemUtilities::joinPath(xdir, "tplang");
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
             Subprocess::W32_HIDE_WINDOW, ProcessPriority::PRIORITY_LOW);

  // Copy output to log
  logCopier = new AsyncCopyStreamToLog(proc->getStream(1));
  logCopier->start();

  // Parse GCode
  SmartPointer<istream> stream = proc->getStream(pipe);
  // Change file name so GCode error messages make sense
  interpGCode(InputSource(*stream, "<generated gcode>"));

  // Wait for Subprocess
  if (proc->waitFor(5, 10)) errors++;

  // Stop the log copier
  logCopier->join();
  logCopier.release();

  proc.release();
}

void ToolPathTask::runTPLInProcess(const string &filename) {
  Task::begin("Running TPL");

  tplCtx = new tplang::TPLContext(SmartPointer<ostream>::Phony(&cerr), machine);
  tplCtx->setSim(JSON::Reader::parse(StringInputSource(simJSON)));

  // Interpret
  try {
    try {
      tplang::Interpreter(*tplCtx).read(filename);
    } catch (const js::JSInterrupted &e) {
      LOG_WARNING("TPL run interrupted");
    }
    return tplCtx.release();

  } CATCH_ERROR;
  errors++;

  tplCtx.release();
}


void ToolPathTask::runGCode(const string &filename) {
  Task::begin("Running GCode");

  SmartPointer<istream> stream;
  io::filtering_istream filter;

  // Track the file load
  TaskFilter taskFilter(*this, SystemUtilities::getFileSize(filename));
  filter.push(boost::ref(taskFilter));

  // Interpret GCode
  filter.push(io::file_source(filename));
  interpGCode(InputSource(filter, filename));
}


void ToolPathTask::run() {
  // Interpret files
  try {
    for (unsigned i = 0; i < files.size() && !Task::shouldQuit(); i++) {
      const string &filename = files[i];

      if (!SystemUtilities::exists(filename)) continue;
      bool isTPL = String::endsWith(filename, ".tpl");

      if (isTPL) runTPLInProcess(filename);
      else runGCode(filename);
    }
  } CATCH_ERROR;
}


void ToolPathTask::interrupt() {
  Task::interrupt();
  if (proc.isSet()) try {proc->kill(true);} CATCH_ERROR;
  if (logCopier.isSet()) logCopier->stop();
  if (tplCtx.isSet()) tplCtx->interrupt();
}
