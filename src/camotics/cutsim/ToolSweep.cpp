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

#include "ToolSweep.h"

#include "Sweep.h"

#include <camotics/sim/ToolTable.h>

#include <cbang/log/Logger.h>
#include <cbang/time/TimeInterval.h>

#include <algorithm>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolSweep::ToolSweep(const SmartPointer<ToolPath> &path, real startTime,
                     real endTime) :
  path(path), startTime(startTime), endTime(endTime) {

  if (endTime < startTime) {
    swap(startTime, endTime);
    swap(this->startTime, this->endTime);
  }

  if (path->empty()) return;

  int firstMove = path->find(startTime);
  int lastMove = path->find(endTime);

  if (lastMove == -1) lastMove = path->size() - 1;
  if (firstMove == -1) firstMove = lastMove + 1;

  real duration = path->at(lastMove).getEndTime() - startTime;

  LOG_DEBUG(1, "Times: start=" << TimeInterval(startTime) << " end="
            << TimeInterval(startTime + duration) << " duration="
            << TimeInterval(duration));
  LOG_DEBUG(1, "Moves: first=" << firstMove << " last=" << lastMove);

  ToolTable &tools = path->getTools();
  vector<Rectangle3R> bboxes;
  unsigned boxes = 0;

  // Gather nodes in a list
  for (int i = firstMove; i <= lastMove; i++) {
    const Move &move = path->at(i);
    unsigned tool = move.getTool();

    if (sweeps.size() <= tool) sweeps.resize(tool + 1);
    if (sweeps[tool].isNull()) sweeps[tool] = tools.get(tool).getSweep();

    Vector3R startPt = move.getPtAtTime(startTime);
    Vector3R endPt = move.getPtAtTime(endTime);

    sweeps[tool]->getBBoxes(startPt, endPt, bboxes);

    for (unsigned j = 0; j < bboxes.size(); j++)
      insert(&move, bboxes[j]);

    boxes += bboxes.size();
    bboxes.clear();
  }

  finalize(); // Finalize MoveLookup

  LOG_DEBUG(1, "AABBTree boxes=" << boxes << " height=" << getHeight());
}


bool ToolSweep::cull(const Rectangle3R &r) const {
  if (change.isNull()) return false;
  return !change->intersects(r);
}


namespace {
  struct move_sort {
    bool operator()(const Move *a, const Move *b) const {
      return a->getStartTime() < b->getStartTime();
    }
  };
}


real ToolSweep::depth(const Vector3R &p) const {
  vector<const Move *> moves;
  collisions(p, moves);

  // Eariler moves first
  sort(moves.begin(), moves.end(), move_sort());

  real d2 = -numeric_limits<real>::max();

  for (unsigned i = 0; i < moves.size(); i++) {
    const Move &move = *moves[i];

    if (move.getEndTime() < startTime || endTime < move.getStartTime())
      continue;

    Vector3R startPt = move.getPtAtTime(startTime);
    Vector3R endPt = move.getPtAtTime(endTime);

    real sd2 = sweeps[move.getTool()]->depth(startPt, endPt, p);
    if (0 <= sd2) return sd2; // Approx 5% faster
    if (d2 < sd2) d2 = sd2;
  }

  return d2;
}
