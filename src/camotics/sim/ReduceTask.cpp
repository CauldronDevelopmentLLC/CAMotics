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

#include "ReduceTask.h"

#include <camotics/contour/Surface.h>

#include <cbang/Catch.h>
#include <cbang/time/Timer.h>
#include <cbang/time/TimeInterval.h>
#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ReduceTask::ReduceTask(const SmartPointer<Surface> &surface) :
  surface(surface) {}


void ReduceTask::run() {
  LOG_INFO(1, "Reducing mesh");

  double startTime = Timer::now();
  double startCount = surface->getTriangleCount();

  surface->reduce(*this);

  unsigned count = surface->getTriangleCount();
  double r = (double)(startCount - count) / startCount * 100;

  LOG_INFO(1, "Time: " << TimeInterval(Timer::now() - startTime)
           << String::printf(" Triangles: %u Reduction: %0.2f%%", count, r));
}
