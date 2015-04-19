/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>

#include <openscam/Application.h>
#include <openscam/sim/Machine.h>
#include <openscam/gcode/Interpreter.h>
#include <openscam/cutsim/ToolPath.h>

#include <iostream>

using namespace std;
using namespace OpenSCAM;


namespace OpenSCAM {
  class EvalApp : public Application, public Machine {
    Controller controller;
    ToolPath path;

  public:
    EvalApp() :
      Application("OpenSCAM Eval"), Machine(options),
      controller(*this), path(controller.getToolTable()) {}

    // From Application
    void run() {
      Application::run();
      path.print(cout);
    }

    // From cb::Reader
    void read(const cb::InputSource &source)
    {Interpreter(controller).read(source);}

    // From Machine
    void move(const Move &move) {
      Machine::move(move);
      path.add(move);
    }
  };
}


int main(int argc, char *argv[]) {
  return cb::doApplication<OpenSCAM::EvalApp>(argc, argv);
}
