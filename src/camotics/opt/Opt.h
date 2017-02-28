/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#pragma once


#include "Path.h"

#include <camotics/Geom.h>
#include <camotics/Task.h>
#include <camotics/sim/ToolTable.h>

#include <cbang/SmartPointer.h>


namespace CAMotics {
  class ToolPath;
  class AnnealState;

  class Opt : public Task {
    unsigned cutCount;

    typedef std::vector<Path> paths_t;
    paths_t paths;

    unsigned iterations; ///< Iterations per annealing round
    unsigned runs;       ///< Number of optimization runs
    double heatTarget;   ///< Stop heating the system when the average cost
                         ///< reaches this ratio of the starting cost, after a
                         ///< brief greedy optimization
    double minTemp;      ///< Stop opt if temperature drops below this level
    double heatRate;     ///< Rate to heat up system as ratio of current temp
    double coolRate;     ///< Rate to cool system between rounds as ratio of
                         ///< current temp
    double reheatRate;   ///< Rate to reheat system as ratio of current temp
                         ///< Reheating occurs when best cost improved
    unsigned timeout;    ///< Stop opt if no improvement in this many seconds
    double zSafe;        ///< Safe Z height

    ToolTable tools;
    cb::SmartPointer<ToolPath> path;

  public:
    Opt(const ToolPath &path);

    const cb::SmartPointer<ToolPath> &getPath() const {return path;}

    // From Task
    void run();

    double computeCost() const;

    void add(const Move &move);
    double optimize();
    void extract(ToolPath &path) const;

  protected:
    double round(double T, unsigned iterations, AnnealState &current,
                 AnnealState &best);
  };
}
