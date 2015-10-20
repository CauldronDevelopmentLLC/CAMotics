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

#include <camotics/CommandLineApp.h>
#include <camotics/cutsim/ToolPath.h>
#include <camotics/gcode/Interpreter.h>
#include <camotics/gcode/Printer.h>
#include <camotics/gcode/Parser.h>

#include <camotics/machine/MachinePipeline.h>
#include <camotics/machine/MachineState.h>
#include <camotics/machine/MachineLinearizer.h>
#include <camotics/machine/MachineUnitAdapter.h>
#include <camotics/machine/GCodeMachine.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>

#include <iostream>

using namespace std;
using namespace cb;
using namespace CAMotics;


namespace CAMotics {
  class GCodeTool : public CommandLineApp {
    MachinePipeline pipeline;
    SmartPointer<Controller> controller;

    bool linearize;
    bool parseOnly;

  public:
    GCodeTool() :
      CommandLineApp("CAMotics GCode Tool"), linearize(true), parseOnly(false) {

      cmdLine.addTarget("linearize", linearize,
                        "Convert all moves to straight line movements.");
      cmdLine.addTarget("parse", parseOnly,
                        "Only parse the GCode, don't evaluate it.");
    }


    // From Application
    void run() {
      if (!parseOnly) {
        pipeline.add(new MachineUnitAdapter(defaultUnits, outputUnits));
        if (linearize) pipeline.add(new MachineLinearizer);
        pipeline.add(new GCodeMachine(*stream, outputUnits));
        pipeline.add(new MachineState);

        controller = new Controller(pipeline);
      }

      Application::run();
      cout << flush;
    }


    // From cb::Reader
    void read(const InputSource &source) {
      if (parseOnly) {
        Printer printer(*stream);
        Parser().parse(source, printer);

      } else {
        pipeline.start();
        Interpreter(*controller).read(source);
        pipeline.end();
      }
    }
  };
}


int main(int argc, char *argv[]) {
  return doApplication<CAMotics::GCodeTool>(argc, argv);
}
