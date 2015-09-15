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

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/os/SystemUtilities.h>

#include <camotics/Application.h>
#include <camotics/cutsim/CutSim.h>
#include <camotics/cutsim/Project.h>
#include <camotics/stl/STLWriter.h>
#include <camotics/contour/Surface.h>

#include <iostream>
#include <limits>

// This causes Windows to not automatically create a console
#ifdef _WIN32
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

using namespace cb;
using namespace std;
using namespace CAMotics;


namespace CAMotics {
  bool is_xml(const std::string &filename) {
    try {
      if (!SystemUtilities::exists(filename))
        return SystemUtilities::extension(filename) == "xml";

      SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::in);

      while (true) {
        int c = stream->peek();
        if (c == '<') return true;
        else if (isspace(c)) stream->get(); // Next
        else return false; // Not XML
      }

    } CATCH_WARNING;

    return false;
  }


  class SimApp : public Application {
    double time;
    bool reduce;
    bool binary;
    string resolution;

    string input;
    SmartPointer<ostream> output;

    Project project;
    CutSim cutSim;

  public:
    SimApp() :
      Application("CAMotics Sim"), time(0),
      reduce(true), binary(true), project(options), cutSim(options) {

      cmdLine.setUsageArgs
        ("[OPTIONS] <project.xml | input.gcode | input.tpl> <output.stl>");

      cmdLine.setAllowConfigAsFirstArg(false);
      cmdLine.setAllowPositionalArgs(true);

      options.pushCategory("Simulation");
      options.addTarget("time", time, "Simulation end time in seconds.  "
                        "A value of zero simulates the entire path.");
      options.addTarget("reduce", reduce, "Reduce cut workpiece.");
      options.addTarget("binary", binary,
                        "Output binary STL, otherwise ASCII.");
      options.addTarget("resolution", resolution, "Valid values are 'low', "
                        "'medium', 'high' or a decimal value.");
      options.popCategory();

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
        THROWS("Too many (" << args.size() << ") positional arguments.");
      if (args.size() < 1)
        THROW("Missing project, GCode or TPL input argument.");
      if (args.size() < 2) THROW("Missing STL output argument.");

      input = args[0];
      output = SystemUtilities::oopen(args[1]);

      return 0;
    }


    void run() {
      // Open project
      if (is_xml(input)) project.load(input);
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
      SmartPointer<ToolPath> path = cutSim.computeToolPath(project);

      // Configure simulation
      project.updateAutomaticWorkpiece(*path);

      // Simulate
      if (!time) time = numeric_limits<double>::max();
      SmartPointer<Simulation> sim = project.makeSim(path, time);
      SmartPointer<Surface> surface;
      if (!shouldQuit()) surface = cutSim.computeSurface(sim);

      // Reduce
      if (reduce && !shouldQuit()) cutSim.reduceSurface(*surface);

      // Export surface
      if (shouldQuit())
        surface->writeSTL(*output, binary, "CAMotics Surface",
                          sim->computeHash());
    }


    void requestExit() {
      Application::requestExit();
      cutSim.interrupt();
    }
  };
}


int main(int argc, char *argv[]) {
  return doApplication<CAMotics::SimApp>(argc, argv);
}
