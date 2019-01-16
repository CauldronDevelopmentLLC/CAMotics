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

#include <gcode/plan/Planner.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/json/Writer.h>
#include <cbang/json/Reader.h>
#include <cbang/io/StringInputSource.h>
#include <cbang/time/TimeInterval.h>

#include <iostream>

using namespace std;
using namespace cb;
using namespace GCode;


class PlannerApp : public CAMotics::Application {
  PlannerConfig config;

public:
  PlannerApp() :
    CAMotics::Application("CAMotics GCode Path Planner") {
    cmdLine.add("json", "JSON configuration or configuration file"
                )->setType(Option::STRING_TYPE);
  }


  // From Application
  int init(int argc, char *argv[]) {
    int ret = Application::init(argc, argv);
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


  // From cb::Reader
  void read(const InputSource &source) {
    Planner planner;
    planner.load(source, config);

    JSON::Writer writer(cout);

    writer.beginList();

    while (!shouldQuit() && planner.hasMore()) {
      writer.beginAppend();
      planner.setActive(planner.next(writer));

      // Cannot synchronize with actual machine so fake it.
      if (planner.isSynchronizing()) planner.synchronize(0);
    }

    writer.endList();

    cout << endl;
    cerr << "Total time: " << TimeInterval(planner.getTime()) << endl;
    cerr << "Total dist: " << planner.getDistance() / 1000 << "m" << endl;
  }
};


int main(int argc, char *argv[]) {
  return doApplication<PlannerApp>(argc, argv);
}
