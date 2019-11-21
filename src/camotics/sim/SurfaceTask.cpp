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

#include "SurfaceTask.h"

#include <camotics/sim/SimulationRun.h>
#include <camotics/contour/Surface.h>

#include <cbang/String.h>
#include <cbang/time/Timer.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/log/Logger.h>

using namespace cb;
using namespace CAMotics;


SurfaceTask::SurfaceTask(const Simulation &sim) :
  simRun(new SimulationRun(sim)) {}


SurfaceTask::SurfaceTask(const SmartPointer<SimulationRun> &simRun) :
  simRun(simRun) {}


SurfaceTask::~SurfaceTask() {}


void SurfaceTask::run() {
  double startTime = Timer::now();

  surface = simRun->compute(*this);

  // Time
  if (shouldQuit()) {
    LOG_INFO(1, "Render aborted");
    return;
  }

  // Done
  double delta = Timer::now() - startTime;
  unsigned triangles = surface->getTriangleCount();
  LOG_INFO(1, "Time: " << TimeInterval(delta)
           << " Triangles: " << triangles
           << " Triangles/sec: " << String::printf("%0.2f", triangles / delta));
}
