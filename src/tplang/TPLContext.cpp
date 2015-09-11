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

#include "TPLContext.h"
#include "GCodeModule.h"
#include "MatrixModule.h"
#include "DXFModule.h"
#include "ClipperModule.h"

#include <cbang/Exception.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace cb;
using namespace tplang;


TPLContext::TPLContext(ostream &out, MachineInterface &machine,
                       const CAMotics::ToolTable &tools) :
  js::Environment(out), machine(machine), tools(tools) {

  // Add modules
  SmartPointer<GCodeModule> gcodeMod = new GCodeModule(*this);
  addModule(gcodeMod);
  gcodeMod->define(*this);

  SmartPointer<MatrixModule> matrixMod = new MatrixModule(*this);
  addModule(matrixMod);
  matrixMod->define(*this);

  SmartPointer<ClipperModule> clipperMod = new ClipperModule;
  addModule(clipperMod);
  clipperMod->define(*this);

  set("_dxf", addModule(new DXFModule(*this)));

  // Add TPL_PATH search paths
  const char *paths = SystemUtilities::getenv("TPL_PATH");
  if (paths) addSearchPaths(paths);

  // Add HOME search path
  const char *home = SystemUtilities::getenv("HOME");
  if (home) addSearchPaths(string(home) + "/.tpl_lib");

  // Add system search paths
  addSearchPaths("/usr/share/camotics/tpl_lib");
  string exeDir =
    SystemUtilities::dirname(SystemUtilities::getExecutablePath());
  addSearchPaths(exeDir + "/tpl_lib");
#ifdef __APPLE__
  addSearchPaths(exeDir + "/Resources/tpl_lib");
#endif

  // Add .tpl to search extensions
  clearSearchExtensions();
  addSearchExtensions("/package.json .tpl .js .json");
}


js::Module &TPLContext::addModule(const SmartPointer<js::Module> &module) {
  modules.push_back(module);
  return *module;
}
