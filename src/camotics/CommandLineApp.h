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

#ifndef CAMOTICS_COMMAND_LINE_APP_H
#define CAMOTICS_COMMAND_LINE_APP_H

#include "Application.h"
#include "Units.h"

#include <iostream>


namespace CAMotics {
  class CommandLineApp : public Application {
    std::string out;
    bool force;

  protected:
    Units outputUnits;
    Units defaultUnits;

    cb::SmartPointer<std::ostream> stream;

  public:
    CommandLineApp(const std::string &name);

    // From cb::Application
    int init(int argc, char *argv[]);

    int metricAction(cb::Option &opt);
    int imperialAction(cb::Option &opt);
  };
}

#endif // CAMOTICS_COMMAND_LINE_APP_H
