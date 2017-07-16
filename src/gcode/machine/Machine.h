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


#include "MachinePipeline.h"

#include <gcode/MoveStream.h>


namespace GCode {
  class Move;
  class MachineMatrix;

  class Machine : public MachinePipeline, public MoveStream {
    MoveStream &stream;

    // TODO Load machine configuration, ramp up/down, rapid feed, etc.
    double rapidFeed;

  public:
    Machine(MoveStream &stream, double rapidFeed = 1000,
            double maxArcError = 0.01);

    using MachineAdapter::move;

    // From MoveStream
    void move(Move &move);
  };
}
