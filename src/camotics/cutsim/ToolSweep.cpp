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

#include <algorithm>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolSweep::ToolSweep(const SmartPointer<ToolPath> &path, real time) :
  path(path), time(time ? time : std::numeric_limits<real>::max()) {
  ToolTable &tools = path->getTools();
  vector<Rectangle3R> bboxes;
  AABB *nodes = 0;
  unsigned boxes = 0;

  // Gather nodes in a list
  for (ToolPath::const_iterator it = path->begin(); it != path->end(); it++) {
    const Move &move = *it;
    unsigned tool = move.getTool();

    if (sweeps.size() <= tool) sweeps.resize(tool + 1);
    if (sweeps[tool].isNull()) sweeps[tool] = tools.get(tool).getSweep();

    sweeps[tool]->getBBoxes(move.getStartPt(), move.getEndPt(), bboxes);

    for (unsigned i = 0; i < bboxes.size(); i++)
      nodes = (new AABB(&move, bboxes[i]))->prepend(nodes);

    boxes += bboxes.size();
    bboxes.clear();
  }

  // Partition nodes
  partition(nodes);

  LOG_INFO(3, "AABBTree boxes=" << boxes << " leaves=" << getLeafCount()
           << " height=" << getHeight());
}


void ToolSweep::getChangeBounds(vector<Rectangle3R> &bboxes, real startTime,
                                real endTime) const {
  if (endTime < startTime) swap(startTime, endTime);
  int firstMove = path->find(startTime);
  int lastMove = path->find(endTime);

  if (firstMove == -1) return;
  if (lastMove == -1) lastMove = path->size() - 1;

  for (int i = firstMove; i <= lastMove; i++) {
    const Move &move = path->at(i);
    unsigned tool = move.getTool();

    Vector3R startPt =
      i == firstMove ? move.getPtAtTime(startTime) : move.getStartPt();
    Vector3R endPt =
      i == lastMove ? move.getPtAtTime(endTime) : move.getEndPt();

    sweeps[tool]->getBBoxes(startPt, endPt, bboxes);
  }
}


bool ToolSweep::contains(const Vector3R &p) {
  vector<const Move *> moves;
  collisions(p, time, moves);

  for (unsigned i = 0; i < moves.size(); i++) {
    const Move &move = *moves[i];
    if (time < move.getStartTime()) continue;
    if (sweeps[move.getTool()]->contains(move, p, time)) return true;
  }

  return false;
}
