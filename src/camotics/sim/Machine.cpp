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

#include "Machine.h"

#include <tplang/MachineState.h>
#include <tplang/MoveSink.h>
#include <tplang/MachineMatrix.h>
#include <tplang/MachineLinearizer.h>
#include <tplang/MachineUnitAdapter.h>

#include <camotics/cutsim/Move.h>

#include <cbang/log/Logger.h>
#include <cbang/os/SystemUtilities.h>

using namespace cb;
using namespace std;
using namespace tplang;
using namespace CAMotics;


Machine::Machine(double rapidFeed) : time(0), distance(0) {
  add(new MachineUnitAdapter);
  add(new MachineLinearizer);
  add(new MachineMatrix);
  add(new MoveSink(*this, rapidFeed));
  add(new MachineState);
}


void Machine::move(const Move &move) {
  // Measure
  distance += move.getDistance();
  time += move.getTime();

  LOG_INFO(3, "Machine: Move to " << move.getEndPt());
}
