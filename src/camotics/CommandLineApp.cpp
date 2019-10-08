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

#include "CommandLineApp.h"

#include <gcode/machine/MachineState.h>
#include <gcode/machine/MachineLinearizer.h>
#include <gcode/machine/MachineUnitAdapter.h>
#include <gcode/machine/GCodeMachine.h>
#include <gcode/machine/JSONMachineWriter.h>

#include <cbang/config/MinConstraint.h>
#include <cbang/os/SystemUtilities.h>

#include <boost/version.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
namespace io = boost::iostreams;

#if BOOST_VERSION < 104400
#define BOOST_CLOSE_HANDLE true
#else
#define BOOST_CLOSE_HANDLE io::close_handle
#endif

using namespace CAMotics;
using namespace GCode;
using namespace cb;
using namespace std;


CommandLineApp::CommandLineApp(const string &name, hasFeature_t hasFeature) :
  Application(name, hasFeature) {
  cmdLine.addTarget("out", out, "Output filename or '-' to write "
                    "to the standard output stream");
  cmdLine.addTarget("force", force, "Force overwriting output file", 'f');

  cmdLine.add("metric", 0, this, &CommandLineApp::metricAction,
              "Output in metric units.")->setType(Option::BOOLEAN_TYPE);
  cmdLine.add("imperial", 0, this, &CommandLineApp::imperialAction,
              "Output in imperial units.")->setType(Option::BOOLEAN_TYPE);

  cmdLine.addTarget("units", outputUnits, "Set output units.");
  cmdLine.addTarget("default-units", defaultUnits,
                    "Units assumed at the start.");

  cmdLine.addTarget("max-arc-error", maxArcError,
                    "The maximum allowed error, in length units, when "
                    "estimating arcs with line segments.  Default value is "
                    "in mm.");
  cmdLine.addTarget("linearize", linearize,
                    "Convert all moves to straight line movements.");

  cmdLine.addTarget("json-out", jsonOut, "Output in JSON format.");
  cmdLine.addTarget("json-precision", jsonPrecision,
                    "JSON output numerical precision.");
  cmdLine.addTarget("json-location", jsonLocation,
                    "Output source location information in JSON.");

  Option &opt = *cmdLine.add("pipe", "Specify a output file descriptor, "
                             "overrides the 'out' option");
  opt.setType(Option::INTEGER_TYPE);
  opt.setConstraint(new MinConstraint<int>(0));
}


bool CommandLineApp::_hasFeature(int feature) {
  switch (feature) {
  case FEATURE_SIGNAL_HANDLER: return false;
  default: return Application::_hasFeature(feature);
  }
}


int CommandLineApp::init(int argc, char *argv[]) {
  int ret = Application::init(argc, argv);
  if (ret == -1) return ret;

  if (cmdLine["--pipe"].hasValue()) {
#ifdef _WIN32
    typedef void *handle_t;
#else
    typedef int handle_t;
#endif

    handle_t pipe = (handle_t)cmdLine["--pipe"].toInteger();
    stream = new io::stream<io::file_descriptor>(pipe, BOOST_CLOSE_HANDLE);

  } else if (out == "-") stream = SmartPointer<ostream>::Phony(&cout);
  else {
    if (SystemUtilities::exists(out) && !force)
      THROW("File '" << out << "' already exists");
    stream = SystemUtilities::oopen(out);
  }

  return ret;
}


void CommandLineApp::run() {
  Application::run();
  stream->flush();
}


void CommandLineApp::build(GCode::MachinePipeline &pipeline) {
  pipeline.add(new MachineUnitAdapter(defaultUnits, outputUnits));
  if (linearize) pipeline.add(new MachineLinearizer);
  if (jsonOut) pipeline.add(new JSONMachineWriter
                            (*stream, outputUnits, jsonLocation, 0, false, 2,
                             jsonPrecision));
  else pipeline.add(new GCodeMachine(stream, outputUnits));
  pipeline.add(new MachineState);

  pipeline.set("_max_arc_error", maxArcError, outputUnits);
}


int CommandLineApp::metricAction(Option &opt) {
  outputUnits = opt.toBoolean() ? GCode::Units::METRIC : GCode::Units::IMPERIAL;
  return 0;
}


int CommandLineApp::imperialAction(Option &opt) {
  outputUnits = opt.toBoolean() ? GCode::Units::IMPERIAL : GCode::Units::METRIC;
  return 0;
}
