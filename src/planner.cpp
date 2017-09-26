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

#include <camotics/CommandLineApp.h>
#include <gcode/ToolPath.h>
#include <gcode/interp/Interpreter.h>
#include <gcode/parse/Parser.h>

#include <gcode/machine/MachinePipeline.h>
#include <gcode/machine/MachineState.h>
#include <gcode/machine/MachineLinearizer.h>
#include <gcode/machine/MachineUnitAdapter.h>
#include <gcode/plan/LinePlanner.h>
#include <gcode/plan/TinyGPlanner.h>
#include <gcode/plan/PlannerJSONMoveSink.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/json/Writer.h>
#include <cbang/json/Reader.h>
#include <cbang/io/StringInputSource.h>

#include <iostream>

using namespace std;
using namespace cb;
using namespace GCode;


class PlannerApp : public CAMotics::CommandLineApp {
  PlannerConfig config;
  MachinePipeline pipeline;
  SmartPointer<Controller> controller;

public:
  PlannerApp() : CAMotics::CommandLineApp("CAMotics GCode Path Planner") {
    cmdLine.add("json", "JSON configuration or configuration file"
                )->setType(Option::STRING_TYPE);
    cmdLine.add("tinyg", "Use the TinyG planner")->setDefault(false);
  }


  // From Application
  int init(int argc, char *argv[]) {
    int ret = CommandLineApp::init(argc, argv);
    if (ret == -1) return ret;

    if (cmdLine["--json"].hasValue()) {
      string s = String::trim(cmdLine["--json"].toString());

      if (!s.empty()) {
        SmartPointer<InputSource> source;

        if (s[0] == '{') source = new StringInputSource(s);
        else source = new InputSource(s);

        config.read(*JSON::Reader::parse(*source));
      }
    }

    return ret;
  }


  void run() {
    config.units = outputUnits;

    JSON::Writer writer(*stream);
    PlannerJSONMoveSink plannerSink(writer);

    SmartPointer<MachineAdapter> planner;
    if (cmdLine["--tinyg"].toBoolean())
      planner = new TinyGPlanner(plannerSink, config);
    else planner = new LinePlanner(plannerSink, config);

    pipeline.add(new MachineUnitAdapter(defaultUnits, outputUnits));
    pipeline.add(new MachineLinearizer);
    pipeline.add(planner);
    pipeline.add(new MachineState);

    controller = new Controller(pipeline);

    Application::run();
    cout << flush;
  }


  // From cb::Reader
  void read(const InputSource &source) {
    pipeline.start();
    Interpreter(*controller).read(source);
    pipeline.end();
  }
};


int main(int argc, char *argv[]) {
  return doApplication<PlannerApp>(argc, argv);
}
