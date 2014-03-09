/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include <openscam/Application.h>
#include <openscam/cutsim/ToolPath.h>
#include <openscam/sim/ToolTable.h>
#include <openscam/sim/Machine.h>

#include <tplang/TPLContext.h>
#include <tplang/MachinePipeline.h>
#include <tplang/MachineState.h>
#include <tplang/MachineMatrix.h>
#include <tplang/MachineLinearizer.h>
#include <tplang/MachineUnitAdapter.h>
#include <tplang/GCodeMachine.h>
#include <tplang/Interpreter.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/js/Javascript.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace tplang;
using namespace OpenSCAM;


namespace OpenSCAM {
  class TPLangApp : public Application {
    ToolTable tools;
    tplang::MachinePipeline pipeline;

  public:
    TPLangApp() :
      Application("Tool Path Language") {

      pipeline.add(new MachineUnitAdapter);
      pipeline.add(new MachineLinearizer);
      pipeline.add(new MachineMatrix);
      pipeline.add(new GCodeMachine(cout));
      pipeline.add(new MachineState);
    }

    // From cb::Reader
    void read(const cb::InputSource &source) {
      tplang::TPLContext ctx(cout, pipeline, tools);

      if (!source.getName().empty() && source.getName()[0] != '<')
        ctx.pushPath(source.getName());

      tplang::Interpreter interp(ctx);

      interp.read(source);
      cout.flush();
    }
  };
}


int main(int argc, char *argv[]) {
  // Look for v8 args after --
  bool foundV8Args = false;

  for (int i = 1; i < argc; i++)
    if (string("--") == argv[i]) {
      vector<char *> args;

      args.push_back(argv[0]);
      for (int j = i + 1; j < argc; j++) args.push_back(argv[j]);

      int v8Argc = argc - i + 1;
      cb::js::Javascript::init(&v8Argc, &args[0]);

      argc = i;
    }

  if (!foundV8Args) cb::js::Javascript::init(0, 0);

  return cb::doApplication<OpenSCAM::TPLangApp>(argc, argv);
}
