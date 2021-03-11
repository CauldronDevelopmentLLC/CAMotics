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

#include <camotics/Application.h>
#include <camotics/sim/Simulation.h>
#include <camotics/sim/CutSim.h>
#include <camotics/project/Project.h>
#include <camotics/contour/Surface.h>

#include <stl/Writer.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/os/SystemInfo.h>
#include <cbang/config.h>

#include <iostream>
#include <limits>

#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
#endif

using namespace cb;
using namespace std;
using namespace CAMotics;


namespace CAMotics {
  class SimApp : public Application {
    double time = 0;
    bool reduce = false;
    bool binary = true;
    RenderMode renderMode;
    string resolution;
    unsigned threads;

    string input;
    SmartPointer<ostream> output;

    Project::Project project;
    CutSim cutSim;

  public:
    SimApp() :
      Application("CAMotics Sim"),
      threads(SystemInfo::instance().getCPUCount()) {

      cmdLine.setUsageArgs
        ("[OPTIONS] <project.camotics | input.gcode | input.tpl> <output.stl>");

      cmdLine.setAllowConfigAsFirstArg(false);
      cmdLine.setAllowPositionalArgs(true);

      cmdLine.addTarget("time", time, "Simulation end time in seconds.  "
                        "A value of zero simulates the entire path.");
      cmdLine.addTarget("reduce", reduce, "Reduce cut workpiece.");
      cmdLine.addTarget("binary", binary,
                        "Output binary STL, otherwise ASCII.");
      cmdLine.addTarget("render-mode", renderMode,
                        "Render surface generation mode.");
      cmdLine.addTarget("resolution", resolution, "Valid values are 'low', "
                        "'medium', 'high' or a decimal value.");
      cmdLine.addTarget("threads", threads, "Number of simulation threads.");

      Logger::instance().setLogTime(false);
      Logger::instance().setLogNoInfoHeader(true);
      Logger::instance().setVerbosity(2);
    }


    // From Application
    int init(int argc, char *argv[]) {
      int ret = Application::init(argc, argv);
      if (ret == -1) return ret;

      vector<string> args = cmdLine.getPositionalArgs();
      if (2 < args.size())
        THROW("Too many (" << args.size() << ") positional arguments.");
      if (args.size() < 1)
        THROW("Missing project, GCode or TPL input argument.");
      if (args.size() < 2) THROW("Missing STL output argument.");

      input = args[0];
      output = SystemUtilities::oopen(args[1]);

      return 0;
    }


    void run() {
      // Open project
      string ext = SystemUtilities::extension(input);
      if (ext == "xml" || ext == "camotics") project.load(input);
      else project.addFile(input); // Assume TPL or G-Code

      // Resolution
      if (!resolution.empty()) {
        ResolutionMode resMode = ResolutionMode::RESOLUTION_MANUAL;
        double res = 0;

        try {
          res = String::parseDouble(resolution);
        } catch (const Exception &e) {}

        if (res) project.setResolution(res);
        else resMode = ResolutionMode::parse(resolution, resMode);

        project.setResolutionMode(resMode);
      }

      // Generate tool path
      SmartPointer<GCode::ToolPath> path = cutSim.computeToolPath(project);

      // Simulate
      Rectangle3D bounds = project.getWorkpiece().getBounds();
      project.getWorkpiece().update(*path);

      Simulation sim(path, 0, 0, bounds, project.getResolution(),
                     time ? time : numeric_limits<double>::max(),
                     renderMode, threads);

      SmartPointer<Surface> surface;
      if (!shouldQuit()) surface = cutSim.computeSurface(sim);

      // Reduce
      if (reduce && !shouldQuit()) cutSim.reduceSurface(surface);

      // Export surface
      if (!shouldQuit())
        surface->writeSTL
          (*output, binary, "CAMotics Surface", sim.computeHash());
    }


    void requestExit() {
      Application::requestExit();
      cutSim.interrupt();
    }
  };
}


int main(int argc, char *argv[]) {
#ifdef HAVE_V8
  cb::gv8::JSImpl::init(0, 0);
#endif
  return doApplication<CAMotics::SimApp>(argc, argv);
}
