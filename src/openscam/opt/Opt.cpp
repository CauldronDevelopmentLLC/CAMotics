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

#include "Opt.h"

#include "AnnealState.h"

#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/config/Options.h>
#include <cbang/time/Time.h>
#include <cbang/os/SystemUtilities.h>

#include <openscam/gcode/Parser.h>
#include <openscam/gcode/Codes.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Opt::Opt(Options &options, ostream &stream) :
  Machine(options), Printer(stream), controller(*this), interp(controller),
  pathCount(0), cutCount(0), iterations(10000), runs(1), heatTarget(1.5),
  minTemp(0.01), heatRate(1.5), coolRate(0.95), reheatRate(2), timeout(10) {

  options.pushCategory("Opt");
  options.addTarget("iterations", iterations,
                    "Number of iterations per annealing round");
  options.addTarget("runs", runs, "Number of optimization runs");
  options.addTarget("heat-target", heatTarget, "Stop heating the "
                    "system when the average cost reaches this ratio of the "
                    "starting cost, after a brief greedy optimization.");
  options.addTarget("min-temp", minTemp, "Stop the optimization if the "
                    "temperature drops below this level.");
  options.addTarget("heat-rate", heatRate, "The rate at which to heat up the "
                    "system as a ratio of the current temperature.");
  options.addTarget("cool-rate", coolRate, "The rate at which to cool the "
                    "system between rounds as a ratio of the current "
                    "temperature.");
  options.addTarget("reheat-rate", reheatRate, "The rate at which to reheat "
                    "the system as a ratio of the current temperature.  "
                    "Reheating occures when the best cost was improved during "
                    "a round.");
  options.addTarget("timeout", timeout, "Stop the optimization if no "
                    "improvement occures with in this many seconds.");
  options.popCategory();
}


double Opt::computeCost() const {
  double cost = 0;

  for (unsigned i = 0; i < groups.size(); i++)
    cost += groups[i]->computeCost();

  return cost;
}


void Opt::read(const InputSource &source) {
  pathCount = 0;
  cutCount = 0;
  groups.clear();
  srand(Time::now());

  // Parse program
  try {
    Parser().parse(source, interp);
  } catch (const EndProgram &) {}

  double initialCost = computeCost();

  LOG_INFO(1, "Groups: " << groups.size());
  LOG_INFO(1, "Paths: " << pathCount);
  LOG_INFO(1, "Cuts: " << cutCount);
  LOG_INFO(1, "Initial cost: " << initialCost);

  double cost = 0;
  for (unsigned i = 0; i < groups.size(); i++) {
    LOG_INFO(1, "Optimizing group " << i);
    cost += optimize(*groups[i]);
  }

  LOG_INFO(1, "Final cost: " << cost);
  LOG_INFO(1, "Improvement: " << 100 - cost / initialCost * 100 << "% or "
           << initialCost / cost << 'x');

  // TODO output modified GCode
}


void Opt::move(const Move &move) {
  if (move.getType() == MoveType::MOVE_CUTTING) {
    if (currentPath.isNull()) currentPath = new Path;
    currentPath->push_back(move);
    cutCount++;

  } else if (!currentPath.isNull()) {
    if (currentGroup.isNull()) {
      currentGroup = new Group;
      groups.push_back(currentGroup);
    }
    currentGroup->push_back(currentPath.adopt());
    pathCount++;
  }
}


void Opt::operator()(const SmartPointer<Block> &block) {
  interp(block);

  // TODO

  if (!block->isDeleted()) {
  }

  Printer::operator()(block);
}


static bool accept(double delta, double T) {
  return
    delta < 0 ? true : (rand() < exp(-delta / (T * 0.00001)) * RAND_MAX);
}


double Opt::round(double T, unsigned iterations, AnnealState &current,
                  AnnealState &best) {
  double average = 0;

  for (unsigned round = 0; round < iterations; round++) {
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
                  << " delta=" << delta << " real delta=" << realDelta);
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


double Opt::optimize(Group &group) {
  AnnealState start(group);
  AnnealState best(group);
  AnnealState veryBest(group);
  AnnealState current(group);

  for (unsigned run = 0; run < runs; run++) {
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
    } while (average < target);

    // Run until cold
    LOG_INFO(1, "Anealing");
    uint64_t lastImprovement = Time::now();

    while (true) {
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
    vector<SmartPointer<Path> > tmp(group.begin(), group.end());
    group.clear();
    for (unsigned i = 0; i < tmp.size(); i++) {
      if (best.flip[best.index[i]]) tmp[best.index[i]]->reverse();
      group.push_back(tmp[best.index[i]]);
    }
  }
  
  return best.cost;
}
