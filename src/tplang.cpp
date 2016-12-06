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
#include <camotics/cutsim/Simulation.h>

#include <camotics/machine/Machine.h>
#include <camotics/machine/MachinePipeline.h>
#include <camotics/machine/MachineState.h>
#include <camotics/machine/MachineMatrix.h>
#include <camotics/machine/MachineLinearizer.h>
#include <camotics/machine/MachineUnitAdapter.h>
#include <camotics/machine/GCodeMachine.h>

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/io/StringInputSource.h>
#include <cbang/json/Reader.h>
#include <cbang/util/DefaultCatch.h>

#include <vector>

using namespace std;
using namespace cb;
using namespace tplang;
using namespace CAMotics;


namespace CAMotics {
  class TPLangApp : public CommandLineApp {
    Simulation sim;
    MachinePipeline pipeline;
    string simJSON;
    string jsImpl;

  public:
    TPLangApp() : CommandLineApp("Tool Path Language Interpreter") {
      cmdLine.addTarget("sim-json", simJSON,
                        "Simulation information in JSON format");
      cmdLine.addTarget("js", jsImpl,
                        "Specify which Javascript implementation to use.  "
                        "Possible values are 'v8' or 'chakra' but which are "
                        "actually available depends on the build.");
    }

    // From CommandLineApp
    int init(int argc, char *argv[]) {
      int ret = CommandLineApp::init(argc, argv);
      if (ret == -1) return ret;

      // Build machine pipeline
      pipeline.add(new MachineUnitAdapter(defaultUnits, outputUnits));
      pipeline.add(new MachineLinearizer);
      pipeline.add(new MachineMatrix);
      pipeline.add(new GCodeMachine(*stream, outputUnits));
      pipeline.add(new MachineState);

      if (!simJSON.empty()) sim.parse(simJSON);

      return ret;
    }


    void requestExit() {
      Application::requestExit();
      // TODO terminate Javascript execution
    }

    // From cb::Reader
    void read(const cb::InputSource &source) {
      tplang::TPLContext ctx(cout, pipeline, sim, jsImpl);
      tplang::Interpreter(ctx).read(source);
      stream->flush();
      cout.flush();
    }
  };
}


#ifdef HAVE_V8
#include <cbang/v8/JSImpl.h>
#endif


int main(int argc, char *argv[]) {
#ifdef HAVE_V8
  // Look for v8 args after --
  bool foundV8Args = false;

  for (int i = 1; i < argc; i++)
    if (string("--") == argv[i]) {
      vector<char *> args;

      args.push_back(argv[0]);
      for (int j = i + 1; j < argc; j++) args.push_back(argv[j]);

      int v8Argc = argc - i + 1;
      gv8::JSImpl::init(&v8Argc, &args[0]);

      argc = i;
    }

  if (!foundV8Args) gv8::JSImpl::init(0, 0);
#endif

  return cb::doApplication<CAMotics::TPLangApp>(argc, argv);
}
