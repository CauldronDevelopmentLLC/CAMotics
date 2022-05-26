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

#include <gcode/plan/PlannerMachine.h>

#include <cbang/config.h>
#include <cbang/Catch.h>

#include <cbang/os/SystemUtilities.h>

#include <cbang/iostream/VectorDevice.h>
#include <cbang/iostream/TeeFilter.h>

#include <cbang/log/AsyncCopyStreamToLog.h>
#include <cbang/io/StringInputSource.h>

#include <cbang/js/JSInterrupted.h>

#include <boost/ref.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;

#include <sstream>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathTask::ToolPathTask(const Project::Project &project,
                           const GCode::PlannerConfig *config) :
  tools(project.getTools()), units(project.getUnits()),
  simJSON(project.toString()), controller(pipeline, tools),
  path(new GCode::ToolPath(tools)) {

  for (unsigned i = 0; i < project.getFileCount(); i++)
    files.push_back(project.getFile(i)->getPath());

  // Save GCode stream
  SmartPointer<ostream>::Phony gcodePtr(&gcode);

  // Create machine pipeline
  pipeline.add(new GCode::MachineUnitAdapter);
  pipeline.add(new GCode::MachineLinearizer);

  // Setup planner
  if (config) pipeline.add(new GCode::PlannerMachine(*config));

  pipeline.add(new GCode::MoveSink(*path));
  if (units != GCode::Units::METRIC)
    pipeline.add(new GCode::MachineUnitAdapter(GCode::Units::METRIC, units));
  pipeline.add(new GCode::GCodeMachine(gcodePtr, units));
  pipeline.add(new GCode::MachineState);
}


ToolPathTask::~ToolPathTask() {interrupt();}


void ToolPathTask::runTPL(const InputSource &src) {
#if !defined(CAMOTICS_NO_TPL) && (defined(HAVE_V8) || defined(HAVE_CHAKRA))
  Task::begin("Running TPL");

  tplCtx =
    new tplang::TPLContext(SmartPointer<ostream>::Phony(&cerr), pipeline);
  tplCtx->setSim(JSON::Reader::parseString(simJSON));

  // Interpret
  try {
    try {
      tplang::Interpreter(*tplCtx).read(src);
    } catch (const js::JSInterrupted &e) {
      LOG_WARNING("TPL run interrupted");
    }
    return tplCtx.release();

  } CATCH_ERROR;
  errors++;

  tplCtx.release();

#else
  THROW("TPL not supported in this build");
#endif
}


void ToolPathTask::runTPL(const string &filename) {
  runTPL(InputSource(filename));
}


void ToolPathTask::runTPLString(const std::string &s) {
  runTPL(StringInputSource(s));
}


void ToolPathTask::runGCode(const InputSource &source) {
  Task::begin("Running GCode");

  GCode::Interpreter interp(controller);
  interp.push(source);
  pipeline.start();

  try {
    while (!Task::shouldQuit() && interp.hasMore() && errors < 32)
      try {
        interp(interp.next());

      } catch (const Exception &e) {
        LOG_ERROR(e);
        errors++;
      }
  } catch (const GCode::EndProgram &) {}
  pipeline.end();
}


void ToolPathTask::runGCode(const string &filename) {
  SmartPointer<istream> stream;
  io::filtering_istream filter;

  // Track the file load
  TaskFilter taskFilter(*this, SystemUtilities::getFileSize(filename));
  filter.push(boost::ref(taskFilter));

  // Interpret GCode
  filter.push(io::file_source(filename));
  runGCode(InputSource(filter, filename));
}


void ToolPathTask::runGCodeString(const string &gcode) {
  SmartPointer<istream> stream;
  io::filtering_istream filter;

  // Track the file load
  TaskFilter taskFilter(*this, gcode.size());
  filter.push(boost::ref(taskFilter));

  // Interpret GCode
  filter.push(io::array_source(gcode.data(), gcode.length()));
  runGCode(InputSource(filter, "<string>"));
}


void ToolPathTask::run() {
  // Interpret files
  try {
    for (unsigned i = 0; i < files.size() && !Task::shouldQuit(); i++) {
      const string &filename = files[i];

      if (!SystemUtilities::exists(filename)) {
        LOG_ERROR("File not found '" << filename << "'");
        continue;
      }

      bool isTPL = String::endsWith(filename, ".tpl");

      if (isTPL) runTPL(filename);
      else runGCode(filename);
    }
  } CATCH_ERROR;
}


void ToolPathTask::interrupt() {
  Task::interrupt();
  if (tplCtx.isSet()) tplCtx->interrupt();
}
