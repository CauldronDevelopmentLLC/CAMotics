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

#include <cbang/util/DefaultCatch.h>
#include <cbang/time/TimeInterval.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ReduceTask::ReduceTask(const Surface &surface) : surface(surface.copy()) {}


void ReduceTask::run() {
  LOG_INFO(1, "Reducing mesh");

  double startCount = surface->getCount();

  Task::begin();
  Task::update(0, "Reducing mesh...");

  surface->reduce(*this);

  unsigned count = surface->getCount();
  double r = (double)(startCount - count) / startCount * 100;

  double delta = Task::end();

  LOG_INFO(1, "Time: " << TimeInterval(delta)
           << String::printf(" Triangles: %u Reduction: %0.2f%%", count, r));
}
