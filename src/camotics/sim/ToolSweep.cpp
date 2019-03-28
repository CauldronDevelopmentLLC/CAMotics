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

#include "ToolSweep.h"

#include "Sweep.h"
#include "ConicSweep.h"
#include "CompositeSweep.h"
#include "SpheroidSweep.h"

#include <gcode/ToolTable.h>

#include <cbang/log/Logger.h>
#include <cbang/time/TimeInterval.h>

#include <algorithm>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolSweep::ToolSweep(const SmartPointer<GCode::ToolPath> &path,
                     double startTime, double endTime) :
  path(path), startTime(startTime), endTime(endTime) {

  if (endTime < startTime) {
    swap(startTime, endTime);
    swap(this->startTime, this->endTime);
  }

  unsigned boxes = 0;

  if (!path->empty()) {
    int firstMove = path->find(startTime);
    int lastMove = path->find(endTime);

    if (lastMove == -1) lastMove = path->size() - 1;
    if (firstMove == -1) firstMove = lastMove + 1;

    double duration = path->at(lastMove).getEndTime() - startTime;

    LOG_DEBUG(1, "Times: start=" << TimeInterval(startTime) << " end="
              << TimeInterval(startTime + duration) << " duration="
              << TimeInterval(duration));
    LOG_DEBUG(1, "GCode::Moves: first=" << firstMove << " last=" << lastMove);

    GCode::ToolTable &tools = path->getTools();
    vector<Rectangle3D> bboxes;

    // Gather nodes in a list
    for (int i = firstMove; i <= lastMove; i++) {
      const GCode::Move &move = path->at(i);
      int tool = move.getTool();

      if (tool < 0) continue;
      if (sweeps.size() <= (unsigned)tool) sweeps.resize(tool + 1);
      if (sweeps[tool].isNull()) sweeps[tool] = getSweep(tools.get(tool));

      Vector3D startPt = move.getPtAtTime(startTime);
      Vector3D endPt = move.getPtAtTime(endTime);

      sweeps[tool]->getBBoxes(startPt, endPt, bboxes);

      for (unsigned j = 0; j < bboxes.size(); j++)
        insert(&move, bboxes[j]);

      boxes += bboxes.size();
      bboxes.clear();
    }
  }

  AABBTree::finalize(); // Finalize MoveLookup

  LOG_DEBUG(1, "AABBTree boxes=" << boxes << " height=" << getHeight());
}


bool ToolSweep::cull(const Rectangle3D &r) const {
  if (change.isNull()) return false;
  return !change->intersects(r);
}


namespace {
  struct move_sort {
    bool operator()(const GCode::Move *a, const GCode::Move *b) const {
      return a->getStartTime() < b->getStartTime();
    }
  };
}


double ToolSweep::depth(const Vector3D &p) const {
  vector<const GCode::Move *> moves;
  collisions(p, moves);

  // Eariler moves first
  sort(moves.begin(), moves.end(), move_sort());

  double d2 = -numeric_limits<double>::max();

  for (unsigned i = 0; i < moves.size(); i++) {
    const GCode::Move &move = *moves[i];

    if (move.getEndTime() < startTime || endTime < move.getStartTime())
      continue;

    Vector3D startPt = move.getPtAtTime(startTime);
    Vector3D endPt = move.getPtAtTime(endTime);

    double sd2 = sweeps[move.getTool()]->depth(startPt, endPt, p);
    if (0 <= sd2) return sd2; // Approx 5% faster
    if (d2 < sd2) d2 = sd2;
  }

  return d2;
}


SmartPointer<Sweep> ToolSweep::getSweep(const GCode::Tool &tool) {
  switch (tool.getShape()) {
  case GCode::ToolShape::TS_CYLINDRICAL:
    return new ConicSweep(tool.getLength(), tool.getRadius(), tool.getRadius());

  case GCode::ToolShape::TS_CONICAL:
    return new ConicSweep(tool.getLength(), tool.getRadius(), 0);

  case GCode::ToolShape::TS_BALLNOSE: {
    SmartPointer<CompositeSweep> composite = new CompositeSweep;
    composite->add
      (new SpheroidSweep(tool.getRadius(), 2 * tool.getRadius()), 0);
    composite->add(new ConicSweep(tool.getLength(), tool.getRadius(),
                                  tool.getRadius()), tool.getRadius());
    return composite;
  }

  case GCode::ToolShape::TS_SPHEROID:
    return new SpheroidSweep(tool.getRadius(), tool.getLength());

  case GCode::ToolShape::TS_SNUBNOSE:
    return new ConicSweep(tool.getLength(), tool.getRadius(),
                          tool.getSnubDiameter() / 2);
  }

  THROW("Invalid tool shape " << tool.getShape());
}
