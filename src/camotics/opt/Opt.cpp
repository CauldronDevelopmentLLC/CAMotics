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

#include "Opt.h"

#include "AnnealState.h"

#include <gcode/machine/MachineState.h>
#include <gcode/machine/MoveSink.h>

#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/time/Time.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


Opt::Opt(const GCode::ToolPath &path) :
  cutCount(0), iterations(10000), runs(1), heatTarget(1.5),
  minTemp(0.01), heatRate(1.5), coolRate(0.95), reheatRate(2), timeout(10),
  zSafe(5), tools(path.getTools()) {

  srand(Time::now()); // Randomize

  for (unsigned i = 0; i < path.size(); i++) add(path[i]);
}


void Opt::run() {
  double initialCost = computeCost();

  LOG_INFO(1, "Optimizing " << paths.size() << " paths with " << cutCount
           << " cuts initial cost " << initialCost);

  double cost = optimize();

  LOG_INFO(1, "Final cost was " << cost << ", an improvement of "
           << 100 - cost / initialCost * 100 << "% or " << initialCost / cost
           << 'x');

  path = new GCode::ToolPath(tools);
  extract(*path);
}


double Opt::computeCost() const {
  double cost = 0;
  paths_t::const_iterator last = paths.end();

  for (paths_t::const_iterator it = paths.begin(); it != paths.end(); it++) {
    if (last != paths.end()) cost += last->costTo(*it);
    last = it;
  }

  return cost;
}


void Opt::add(const GCode::Move &move) {
  if (paths.empty()) paths.push_back(Path());

  bool cutting = move.getType() == GCode::MoveType::MOVE_CUTTING;

  if (cutting) {
    paths.back().push_back(move);
    cutCount++;

  } else if (!paths.back().empty())
    paths.push_back(Path());
}


double Opt::optimize() {
  AnnealState start(paths);
  AnnealState best(paths);
  AnnealState veryBest(paths);
  AnnealState current(paths);

  for (unsigned run = 0; run < runs && !shouldQuit(); run++) {
    best = start;

    // Greedy run
    round(0, iterations, current, best);

    double T = 1;
    double average;
    double target = best.cost * heatTarget;

    // Increase temperature up to target
    LOG_INFO(1, "Heating up");
    do {
      T *= heatRate;
      average = round(T, iterations, current, best);
    } while (average < target && !shouldQuit());

    // Run until cold
    LOG_INFO(1, "Anealing");
    uint64_t lastImprovement = Time::now();

    while (!shouldQuit()) {
      double tempBest = best.cost;
      round(T, iterations, current, best);

      if (best.cost < tempBest) {
        LOG_INFO(1, "Temperature " << T << " Cost " << best.cost);

        lastImprovement = Time::now();
        T *= reheatRate;

      } else T *= coolRate;

      if (T < minTemp) break; // Frozen
      if (timeout < Time::now() - lastImprovement) break;
    }

    if (best.cost < veryBest.cost) veryBest = best;
  }

  best = veryBest;

  // Rearrange vector
  if (best.cost < current.cost) {
    vector<Path> tmp(paths.begin(), paths.end());
    paths.clear();

    for (unsigned i = 0; i < tmp.size(); i++) {
      if (best.flip[best.index[i]]) tmp[best.index[i]].reverse();
      paths.push_back(tmp[best.index[i]]);
    }
  }

  return best.cost;
}


void Opt::extract(GCode::ToolPath &path) const {
  GCode::MoveSink sink(path);
  sink.setNextNode(new GCode::MachineState);

  GCode::Axes position = sink.getPosition();
  sink.move(position, VT_X | VT_Y | VT_Z, true, 0);

  for (paths_t::const_iterator it = paths.begin(); it != paths.end() &&
         !shouldQuit(); it++) {
    Vector3D last = sink.getPosition().getXYZ();
    Vector3D next = it->startPoint();

    sink.changeTool(it->begin()->getTool());
    sink.setFeed(it->begin()->getFeed());
    sink.setSpeed(it->begin()->getSpeed());

    if (last != next) {
      position = sink.getPosition();
      position.setZ(zSafe);
      sink.move(position, VT_Z, true, 0);

      position.setX(next.x());
      position.setY(next.y());
      sink.move(position, VT_X | VT_Y, true, 0);

      position.setZ(next.z());
      sink.move(position, VT_Z, false, 0);
    }

    for (unsigned i = 0; i < it->size(); i++) {
      const GCode::Move &move = it->at(i);

      sink.changeTool(move.getTool());
      sink.setFeed(move.getFeed());
      sink.setSpeed(move.getSpeed());
      sink.move(move.getEnd(), VT_X | VT_Y, false, 0);
    }
  }

  position = sink.getPosition();
  position.setZ(zSafe);
  sink.move(position, VT_Z, true, 0);
}


static bool accept(double delta, double T) {
  return delta < 0 ? true : (rand() < exp(-delta / (T * 0.00001)) * RAND_MAX);
}


double Opt::round(double T, unsigned iterations, AnnealState &current,
                  AnnealState &best) {
  double average = 0;

  for (unsigned round = 0; round < iterations && !shouldQuit(); round++) {
    unsigned first = rand() % best.index.size();
    unsigned second = rand() % best.index.size();
    unsigned mode = rand() % 3;

    if (first == second) continue;
    if (second < first) std::swap(first, second);

    double delta = 0;

    switch (mode) {
    case 0: delta = current.swapDelta(first, second); break;
    case 1: delta = current.reverseDelta(first, second); break;
    case 2: delta = current.flipDelta(first); break;
    }

    if (accept(delta / current.cost, T)) {
      switch (mode) {
      case 0: current.acceptSwap(first, second); break;
      case 1: current.acceptReverse(first, second); break;
      case 2: current.acceptFlip(first); break;
      }

#if 0
      double realDelta = current.computeCost() - current.cost;
      if (realDelta != delta) {
        LOG_DEBUG(1, "first=" << first << " second=" << second
                  << " delta=" << delta << " double delta=" << realDelta);
        delta = realDelta;
      }
#endif

      current.cost += delta;

      // Recalculate to fix rounding errors
      if (current.cost < best.cost) current.cost = current.computeCost();
      if (current.cost < best.cost) best = current;
    }

    average += current.cost;
  }

  average /= iterations;

  LOG_INFO(3, "Temperature " << T << " Cost " << current.cost
           << " Average " << average << " Best " << best.cost);

  return average;
}
