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

#include <gcode/machine/MachinePipeline.h>

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <cbang/config.h>
#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/io/StringInputSource.h>
#include <cbang/json/Reader.h>
#include <cbang/Catch.h>

#include <vector>

using namespace std;
using namespace cb;
using namespace tplang;
using namespace GCode;


class TPLangApp : public CAMotics::CommandLineApp {
  MachinePipeline pipeline;
  string simJSON;
  tplang::TPLContext ctx;

public:
  TPLangApp() : CommandLineApp("Tool Path Language Interpreter"),
                ctx(SmartPointer<ostream>::Phony(&cout), pipeline) {
    cmdLine.addTarget("sim-json", simJSON,
                      "Simulation information in JSON format");
  }

  // From CommandLineApp
  void run() {
    build(pipeline);
    ctx.setStream(stream);

    if (!simJSON.empty())
      ctx.setSim(JSON::Reader::parse(StringInputSource(simJSON)));

    CommandLineApp::run();
  }


  void requestExit() {
    Application::requestExit();
    ctx.interrupt(); // Terminate Javascript execution
  }


  // From cb::Reader
  void read(const cb::InputSource &source) {
    tplang::Interpreter(ctx).read(source);
  }
};


#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
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

  return cb::doApplication<TPLangApp>(argc, argv);
}
