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

#include "Machine.h"

#include "MachineState.h"
#include "MoveSink.h"
#include "MachineMatrix.h"
#include "MachineLinearizer.h"
#include "MachineUnitAdapter.h"

#include <gcode/Move.h>

#include <cbang/log/Logger.h>

using namespace cb;
using namespace std;
using namespace GCode;


Machine::Machine(MoveStream &stream, double rapidFeed, double maxArcError) :
  stream(stream), rapidFeed(rapidFeed) {
  add(new MachineUnitAdapter);
  add(new MachineLinearizer(maxArcError));
  add(new MachineMatrix);
  add(new MoveSink(*this));
  add(new MachineState);
}


void Machine::move(Move &move) {
  if (move.getType() == Move::MOVE_RAPID) move.setFeed(rapidFeed);

  stream.move(move);

  LOG_INFO(3, "Machine: Move to " << move.getEndPt() << "mm");
}
