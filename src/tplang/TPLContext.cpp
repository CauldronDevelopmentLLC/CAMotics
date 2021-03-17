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

#include "TPLContext.h"

#include <cbang/Exception.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/json/Dict.h>

using namespace std;
using namespace cb;
using namespace tplang;


TPLContext::TPLContext(const SmartPointer<ostream> &stream,
                       GCode::MachineInterface &machine, const string &jsImpl) :
  js::Javascript(jsImpl, stream),
  GCode::MachineNode(SmartPointer<GCode::MachineInterface>::Phony(&machine)),
  gcodeMod(*this), matrixMod(*this), dxfMod(*this), stlMod(*this),
  sim(new JSON::Dict) {

  // Add modules
  define(gcodeMod);
  define(matrixMod);
  define(clipperMod);
  define(dxfMod);
  define(stlMod);

  import("gcode", ".");
  import("matrix", ".");
  import("clipper", ".");

  // Add TPL_PATH search paths
  const char *paths = SystemUtilities::getenv("TPL_PATH");
  if (paths) addSearchPaths(paths);

  // Add HOME search path
  const char *home = SystemUtilities::getenv("HOME");
  if (home) addSearchPaths(string(home) + "/.tpl_lib");

  // Add system search paths
  string exeDir =
    SystemUtilities::dirname(SystemUtilities::getExecutablePath());
  addSearchPaths(exeDir + "/tpl_lib");
  addSearchPaths(SystemUtilities::getPathPrefix() + "/share/camotics/tpl_lib");
#ifdef __APPLE__
  addSearchPaths(exeDir + "/../Resources/tpl_lib");
#endif

  // Add .tpl to search extensions
  clearSearchExtensions();
  addSearchExtensions("/package.json .tpl .js .json");
}


void TPLContext::pushPath(const string &path) {Javascript::pushPath(path);}
void TPLContext::popPath() {Javascript::popPath();}
