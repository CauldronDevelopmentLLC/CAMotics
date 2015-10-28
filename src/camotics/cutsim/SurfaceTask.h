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

#ifndef CAMOTICS_SURFACE_TASK_H
#define CAMOTICS_SURFACE_TASK_H

#include <camotics/Task.h>
#include <camotics/cutsim/Simulation.h>
#include <camotics/contour/Surface.h>


namespace CAMotics {
  class AABBTree;

  class SurfaceTask : public Task {
    unsigned threads;
    std::string filename;
    cb::SmartPointer<Simulation> sim;
    cb::SmartPointer<Surface> surface;
    cb::SmartPointer<AABBTree> aabbTree;

  public:
    SurfaceTask(unsigned threads, const std::string &filename,
                const cb::SmartPointer<Simulation> &sim) :
      threads(threads), filename(filename), sim(sim) {}
    ~SurfaceTask();

    const cb::SmartPointer<Surface> &getSurface() const {return surface;}
    const cb::SmartPointer<AABBTree> &getAABBTree() const {return aabbTree;}

    // From Task
    void run();
  };
}

#endif // CAMOTICS_SURFACE_TASK_H

