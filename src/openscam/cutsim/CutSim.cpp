/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "CutSim.h"

#include "Workpiece.h"
#include "ToolPath.h"
#include "CutWorkpiece.h"
#include "Project.h"
#include "Sweep.h"

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <openscam/gcode/Interpreter.h>

#include <limits>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


CutSim::CutSim(Options &options) :
  Machine(options), tools(new ToolTable), controller(*this, tools) {}


CutSim::~CutSim() {}


void CutSim::init(Project &project) {
  // Reset
  Machine::reset();
  controller.reset();

  // Load G code
  path = new ToolPath(*tools);

  // Reload interpret code
  vector<string> files = project.getAbsoluteFiles();
  for (unsigned i = 0; i < files.size(); i++) {
    if (String::endsWith(files[i], ".tpl")) {
      tplang::TPLContext ctx(cout, *this, controller.getToolTable());
      ctx.pushPath(files[i]);
      tplang::Interpreter(ctx).read(files[i]);

    } else Interpreter(controller).read(files[i]); // Assume GCode
  }

  // Update changed Project settings
  project.updateAutomaticWorkpiece(*path);
  project.updateResolution();

  // Setup cut simulation
  Rectangle3R wpBounds = project.getWorkpieceBounds();
  cutWP = new CutWorkpiece(new ToolSweep(path), new Workpiece(wpBounds));
}


void CutSim::reset() {
  Machine::reset();
  controller.reset();
  tools->clear();
  path = 0;
  cutWP = 0;
}


void CutSim::move(const Move &move) {
  Machine::move(move);
  path->add(move);
}
