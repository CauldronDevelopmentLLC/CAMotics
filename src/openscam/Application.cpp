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

#include "Application.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace OpenSCAM;


namespace OpenSCAM {
  namespace BuildInfo {
    void addBuildInfo(const char *category);
  }
}


bool Application::_hasFeature(int feature) {
  switch (feature) {
  case FEATURE_INFO: return true;
  case FEATURE_PRINT_INFO: return false;
  default: return cb::Application::_hasFeature(feature);
  }
}


Application::Application(const string &name) :
  cb::Application(name, _hasFeature) {

  // Default to 'C' locale, otherwise double parsing is messed up.
  if (!cb::SystemUtilities::getenv("LC_NUMERIC"))
    cb::SystemUtilities::setenv("LC_NUMERIC", "C");

  cb::Logger::instance().setScreenStream(cerr);
  cb::Logger::instance().setLogThreadPrefix(true);
  cb::Logger::instance().setLogTime(false);

  if (hasFeature(FEATURE_INFO)) {
    BuildInfo::addBuildInfo("Build");

    // TODO move this stuff out to the build system
    cb::Info &info = cb::Info::instance();
    info.add(name, "Website", "http://openscam.org/", true);
    info.add(name, "Copyright", "(c) 2011-2015");
    info.add(name, "Author", "Joseph Coffland <joseph@openscam.org>");
    info.add(name, "Organization", "Cauldron Development LLC");
  }
}


void Application::run() {
  const vector<string> &args = cmdLine.getPositionalArgs();
  if (args.empty()) read(cb::InputSource(cin, "<stdin>"));
  vector<string>::const_iterator it;
  for (it = args.begin(); it != args.end(); it++) read(*it);
}
