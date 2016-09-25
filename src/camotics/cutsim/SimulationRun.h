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

#ifndef CAMOTICS_SIMULATION_RUN_H
#define CAMOTICS_SIMULATION_RUN_H

#include "Simulation.h"

#include <cbang/SmartPointer.h>


namespace CAMotics {
  class ToolSweep;
  class GridTree;
  class Surface;
  class MoveLookup;
  class Task;


  class SimulationRun {
    Simulation sim;
    cb::SmartPointer<ToolSweep> sweep;
    cb::SmartPointer<GridTree> tree;
    cb::SmartPointer<Surface> surface;

    double lastTime;

  public:
    SimulationRun(const Simulation &sim);
    ~SimulationRun();

    const cb::SmartPointer<Surface> &getSurface() const {return surface;}
    cb::SmartPointer<MoveLookup> getMoveLookup() const;

    void setEndTime(double endTime);

    void compute(const cb::SmartPointer<Task> &task);
  };
}

#endif // CAMOTICS_SIMULATION_RUN_H
