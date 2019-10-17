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
#include <cbang/log/Logger.h>

#include <iostream>

using namespace std;
using namespace cb;
using namespace GCode;


class PlannerApp : public CAMotics::Application {
  PlannerConfig config;
  bool gcode = false;
  unsigned precision = 4;

public:
  PlannerApp() :
    CAMotics::Application("CAMotics GCode Path Planner") {
    cmdLine.add("json", "JSON configuration or configuration file"
                )->setType(Option::STRING_TYPE);
    cmdLine.addTarget("gcode", gcode, "Output GCode instead of plan JSON");
    cmdLine.addTarget("precision", precision,
                      "Decimal places in output numbers");

    Logger::instance().setScreenStream(cerr);
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

    SmartPointer<JSON::Writer> writer;
    if (!gcode) writer = new JSON::Writer(cout, 0, false, 2, precision);

    if (writer.isSet()) writer->beginList();

    while (!shouldQuit() && planner.hasMore()) {
      uint64_t id;

      if (writer.isSet()) {
        writer->beginAppend();
        id = planner.next(*writer);

      } else {
        auto e = planner.next();
        id = e->getNumber("id");

        string type = e->getString("type");
        if (type == "line") {
          if (e->getBoolean("rapid", false)) cout << "G0";
          else cout << "G1";

          auto t = e->get("target");
          for (const char *axis = Axes::AXES; *axis; axis++) {
            string axisName = string(1, tolower(*axis));
            if (t->hasNumber(axisName))
              cout << ' ' << *axis << String(t->getNumber(axisName), precision);
          }

          cout << '\n';

        } else if (type == "set") {
          string name = e->getString("name");
          double value = e->getNumber("value");

          if (name == "_feed")
            cout << 'F' << String(value, precision) << '\n';
          else if (name == "tool") cout << "M6 T" << (unsigned)value << '\n';
          else if (name == "speed")
            cout << 'S' << String(value, precision) << '\n';
        }

        // TODO support other GCode output
      }

      planner.setActive(id);

      // Cannot synchronize with actual machine so fake it.
      if (planner.isSynchronizing()) planner.synchronize(0);
    }

    if (writer.isSet()) writer->endList();

    cout << endl;
    cerr << "Total time: " << TimeInterval(planner.getTime()) << endl;
    cerr << "Total dist: " << planner.getDistance() / 1000 << "m" << endl;
  }
};


int main(int argc, char *argv[]) {
  return doApplication<PlannerApp>(argc, argv);
}
