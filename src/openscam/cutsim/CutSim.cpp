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
#include "Sweep.h"

#include <tplang/TPLContext.h>
#include <tplang/Interpreter.h>

#include <openscam/contour/Surface.h>
#include <openscam/gcode/Interpreter.h>
#include <openscam/render/Renderer.h>
#include <openscam/sim/Controller.h>

#include <cbang/js/Javascript.h>
#include <cbang/os/SystemInfo.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/util/DefaultCatch.h>

#include <limits>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


CutSim::CutSim(Options &options) :
  Machine(options), threads(SystemInfo::instance().getCPUCount()),
  task(SmartPointer<Task>::ProtectedNull(this)) {
  options.pushCategory("Simulation");
  options.addTarget("threads", threads, "Number of simulation threads.");
  options.popCategory();
}


CutSim::~CutSim() {}


SmartPointer<ToolPath>
CutSim::computeToolPath(const SmartPointer<ToolTable> &tools,
                        const vector<string> &files) {
  // Setup
  task->begin();
  Machine::reset();
  Controller controller(*this, tools);
  path = new ToolPath(tools);

  // Interpret code
  try {
    for (unsigned i = 0; i < files.size() && !task->shouldQuit(); i++) {
      if (!SystemUtilities::exists(files[i])) continue;

      task->update(0, "Running " + files[i]);

      if (String::endsWith(files[i], ".tpl")) {
        tplang::TPLContext ctx(cout, *this, controller.getToolTable());
        ctx.pushPath(files[i]);
        tplang::Interpreter(ctx).read(files[i]);

      } else Interpreter(controller, task).read(files[i]); // Assume GCode
    }
  } CATCH_ERROR;

  task->end();
  return path.adopt();
}


cb::SmartPointer<Surface>
CutSim::computeSurface(const SmartPointer<ToolPath> &path,
                       const Rectangle3R &bounds, double resolution,
                       double time, bool smooth) {
  // Setup cut simulation
  CutWorkpiece cutWP(new ToolSweep(path, time), new Workpiece(bounds));

  // Render
  Renderer renderer(SmartPointer<Task>::Null(this));
  SmartPointer<Surface> surface = renderer.render(cutWP, threads, resolution);

  // Smooth
  if (smooth && !task->shouldQuit()) {
    task->begin();
    task->update(0, "Smoothing...");
    surface->smooth();
    task->end();
  }

  return surface;
}


void CutSim::interrupt() {
  js::Javascript::terminate(); // End TPL code
  Task::interrupt();
}


void CutSim::move(const Move &move) {
  Machine::move(move);
  path->add(move);
}
