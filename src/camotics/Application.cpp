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

#include "Application.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/Catch.h>

using namespace std;
using namespace CAMotics;


namespace CAMotics {
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


Application::Application(const string &name, hasFeature_t hasFeature) :
  cb::Application(name, hasFeature) {

  // Force 'C' locale, otherwise double parsing is messed up.
  cb::SystemUtilities::setenv("LC_NUMERIC", "C");

  cb::Logger::instance().setScreenStream(cerr);
  cb::Logger::instance().setLogPrefix(true);
  cb::Logger::instance().setLogTime(false);

  if (hasFeature(FEATURE_INFO)) {
    cb::Info::instance().add(name, true);
    BuildInfo::addBuildInfo(name.c_str());
    setVersion(cb::Info::instance().get(name, "Version"));
  }

  cmdLine.setShowKeywordOpts(false);
}


void Application::run() {
  const vector<string> &args = cmdLine.getPositionalArgs();
  if (args.empty()) read(cb::InputSource(cin, "<stdin>"));
  vector<string>::const_iterator it;
  for (it = args.begin(); it != args.end(); it++) read(*it);
}
