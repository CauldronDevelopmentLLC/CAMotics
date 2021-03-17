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

#include <camotics/CommandLineApp.h>

#include <gcode/plan/Planner.h>
#include <gcode/machine/GCodeMachine.h>
#include <gcode/machine/MachinePipeline.h>
#include <gcode/machine/MachineState.h>

#include <cbang/config.h>
#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/json/Writer.h>
#include <cbang/json/Reader.h>
#include <cbang/io/StringInputSource.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/log/Logger.h>

#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
#endif

#include <iostream>

using namespace std;
using namespace cb;
using namespace GCode;


class PlannerApp : public CAMotics::CommandLineApp {
  PlannerConfig config;
  bool gcode = false;
  unsigned precision = 4;

public:
  PlannerApp() :
    CAMotics::CommandLineApp("CAMotics GCode Path Planner") {
    cmdLine.add("json", "JSON configuration or configuration file"
                )->setType(Option::STRING_TYPE);
    cmdLine.addTarget("gcode", gcode, "Output GCode instead of plan JSON");
    cmdLine.addTarget("precision", precision,
                      "Decimal places in output numbers");

    Logger::instance().setScreenStream(cerr);
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

        auto data = JSON::Reader::parse(*source);
        config.read(data->hasDict("planner") ? *data->get("planner") : *data);
      }
    }

    return ret;
  }


  // From cb::Reader
  void read(const InputSource &source) {
    MachinePipeline pipeline;

    Planner planner;
    planner.load(source, config, String::endsWith(source.getName(), ".tpl"));

    SmartPointer<JSON::Writer> writer;
    if (!gcode) writer = new JSON::Writer(cout, 0, false, 2, precision);

    if (writer.isSet()) writer->beginList();
    else {
      pipeline.add(new GCodeMachine(stream, outputUnits));
      pipeline.add(new MachineState);
    }

    while (!shouldQuit() && planner.hasMore()) {
      uint64_t id;

      if (writer.isSet()) {
        writer->beginAppend();
        id = planner.next(*writer);

      } else id = planner.next(pipeline);

      planner.setActive(id); // Flush planner
    }

    if (writer.isSet()) writer->endList();

    cout << endl;
    cerr << "Total time: " << TimeInterval(planner.getTime()) << endl;
    cerr << "Total dist: " << planner.getDistance() / 1000 << "m" << endl;
  }
};


int main(int argc, char *argv[]) {
#ifdef HAVE_V8
  cb::gv8::JSImpl::init(0, 0);
#endif
  return doApplication<PlannerApp>(argc, argv);
}
