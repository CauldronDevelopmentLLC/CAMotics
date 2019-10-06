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

#include <gcode/ControllerImpl.h>
#include <gcode/Printer.h>
#include <gcode/parse/Parser.h>
#include <gcode/interp/Interpreter.h>
#include <gcode/machine/MachinePipeline.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>

#include <iostream>

using namespace std;
using namespace cb;
using namespace GCode;


class GCodeTool : public CAMotics::CommandLineApp {
  MachinePipeline pipeline;
  SmartPointer<Controller> controller;

  bool parseOnly = false;
  bool annotate = false;

public:
  GCodeTool() :
    CAMotics::CommandLineApp("CAMotics GCode Tool") {
    cmdLine.addTarget("parse", parseOnly,
                      "Only parse the GCode, don't evaluate it.");
    cmdLine.addTarget("annotate", annotate, "Annotate parsed GCode.");
  }


  // From Application
  void run() {
    if (!parseOnly) {
      build(pipeline);
      controller = new ControllerImpl(pipeline);
    }

    CommandLineApp::run();
  }


  // From cb::Reader
  void read(const InputSource &source) {
    if (parseOnly) {
      Printer printer(*stream, annotate);
      Parser(source).parse(printer);

    } else {
      pipeline.start();
      Interpreter(*controller).read(source);
      pipeline.end();
    }
  }
};


int main(int argc, char *argv[]) {
  return doApplication<GCodeTool>(argc, argv);
}
