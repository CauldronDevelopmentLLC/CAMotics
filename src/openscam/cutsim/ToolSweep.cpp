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

#include "ToolSweep.h"

#include "Sweep.h"

#include <openscam/sim/ToolTable.h>

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


ToolSweep::ToolSweep(const SmartPointer<ToolPath> &path, real time) :
  path(path), tools(path->getTools()), hitTests(0), time(time) {
  vector<Rectangle3R> bboxes;
  AABB *nodes = 0;
  unsigned boxes = 0;

  // Gather nodes in a list
  for (ToolPath::const_iterator it = path->begin(); it != path->end(); it++) {
    const Move &move = *it;
    unsigned tool = move.getTool();

    if (sweeps.size() <= tool) sweeps.resize(tool + 1);
    if (sweeps[tool].isNull()) sweeps[tool] = tools->get(tool)->getSweep();

    sweeps[tool]->getBBoxes(move.getStartPt(), move.getEndPt(), bboxes);

    for (unsigned i = 0; i < bboxes.size(); i++)
      nodes = (new AABB(&move, bboxes[i]))->prepend(nodes);

    boxes += bboxes.size();
    bboxes.clear();
  }

  // Partition nodes
  partition(nodes);

  LOG_INFO(1, "AABBTree boxes=" << boxes << " leaves=" << getLeafCount()
           << " height=" << getHeight());
}


bool ToolSweep::contains(const Vector3R &p) {
  vector<const Move *> moves;
  collisions(p, time, moves);

  for (unsigned i = 0; i < moves.size(); i++) {
    const Move &move = *moves[i];
    const Sweep &sweep = *sweeps[move.getTool()];

    if (time < move.getStartTime()) continue;

    if (sweep.contains(move.getStartPt(), time < move.getEndTime() ?
                       move.getEndPtAtTime(time) : move.getEndPt(), p))
      return true;
  }

  return false;
}
