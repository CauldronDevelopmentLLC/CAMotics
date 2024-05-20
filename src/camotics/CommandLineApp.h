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

#pragma once


#include "Application.h"

#include <gcode/Units.h>
#include <gcode/machine/MachinePipeline.h>

#include <iostream>


namespace CAMotics {
  class CommandLineApp : public Application {
    std::string out = "-";
    bool force = false;

  protected:
    GCode::Units outputUnits = GCode::Units::METRIC;
    GCode::Units defaultUnits = GCode::Units::METRIC;
    double maxArcError = 0.01;
    bool linearize = false;
    bool jsonOut = false;
    int jsonPrecision = 3;
    bool jsonLocation = false;

    cb::SmartPointer<std::ostream> stream;

  public:
    CommandLineApp(const std::string &name,
                   hasFeature_t hasFeature = CommandLineApp::_hasFeature);

    // From cb::Application
    static bool _hasFeature(int feature);
    int init(int argc, char *argv[]) override;
    void run() override;

    void build(GCode::MachinePipeline &pipeline);

    int metricAction(cb::Option &opt);
    int imperialAction(cb::Option &opt);
  };
}
