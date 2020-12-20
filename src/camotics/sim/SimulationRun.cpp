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

#include "SimulationRun.h"
#include "Simulation.h"

#include <camotics/contour/TriangleSurface.h>
#include <camotics/contour/GridTree.h>
#include <camotics/render/Renderer.h>
#include <camotics/sim/CutWorkpiece.h>

#include <cbang/log/Logger.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/time/Timer.h>

using namespace cb;
using namespace CAMotics;


SimulationRun::SimulationRun(const Simulation &sim) : sim(sim), lastTime(-1) {}


SimulationRun::~SimulationRun() {}


SmartPointer<MoveLookup> SimulationRun::getMoveLookup() const {
  if (!sweep.isNull() && !sweep->getChange().isNull())
    return sweep->getChange();
  return sweep;
}


void SimulationRun::setEndTime(double endTime) {sim.time = endTime;}


SmartPointer<Surface> SimulationRun::compute(Task &task) {
  Rectangle3D bbox;

  double start = Timer::now();
  double simTime = std::min(sim.path->getTime(), sim.time);

  LOG_INFO(1, "Computing surface at " << TimeInterval(simTime));

  if (sweep.isNull()) {
    // GCode::Tool sweep
    sweep = new ToolSweep(sim.path); // Build sweep for entire time period

    // Bounds, increased a little
    bbox = sim.workpiece.getBounds().grow(sim.resolution * 0.9);

    // Grid
    tree = new GridTree(Grid(bbox, sim.resolution));

  } else {
    double minTime = simTime;
    double maxTime = simTime;

    if (lastTime < minTime) minTime = lastTime;
    if (maxTime < lastTime) maxTime = lastTime;

    SmartPointer<MoveLookup> change = new ToolSweep(sim.path, minTime, maxTime);
    sweep->setChange(change);
    bbox = change->getBounds().grow(sim.resolution * 1.1);
  }

  // Set target time
  sweep->setEndTime(simTime);

  // Setup cut simulation
  CutWorkpiece cutWP(sweep, sim.workpiece);

  // Render
  Renderer renderer(task);
  renderer.render(cutWP, *tree, bbox, sim.threads, sim.mode);

  if (task.shouldQuit()) {
    sweep.release();
    tree.release();
    return 0;
  }

  LOG_DEBUG(1, "Render time " << TimeInterval(Timer::now() - start));

  // Extract surface
  lastTime = simTime;
  return new TriangleSurface(*tree);
}
