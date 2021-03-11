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

#include "MoveSink.h"

#include <cbang/log/Logger.h>

using namespace cb;
using namespace GCode;
using namespace GCode;


void MoveSink::seek(port_t port, bool active, bool error) {
  probePending = true;
  return MachineAdapter::seek(port, active, error);
}


void MoveSink::move(const Axes &position, int axes, bool rapid, double time) {
  if (getPosition() != position) {
    MoveType type = rapid ? Move::MOVE_RAPID :
      (probePending ? Move::MOVE_PROBE : Move::MOVE_CUTTING);

    if (!rapid && !getFeed()) {
      setFeed(10);
      LOG_ERROR("Cutting move with zero feed, set feed rate to 10mm/min");
    }

    if (!rapid && get(TOOL_NUMBER, NO_UNITS) < 1) {
      LOG_ERROR("No tool selected, selecting tool 1");
      set(TOOL_NUMBER, 1, NO_UNITS);
    }

    Axes start = getTransforms().transform(getPosition());
    Axes end = getTransforms().transform(position);
    double feed = rapid ? 10000 : getFeed(); // TODO Get rapid feed from machine

    Move move(type, start, end, this->time, get(TOOL_NUMBER, NO_UNITS),
              feed, getSpeed(), getLocation().getStart().getLine(), time);

    this->time += move.getTime();

    stream.move(move);

    probePending = false;
  }

  MachineAdapter::move(position, axes, rapid, time);
}


void MoveSink::arc(const Vector3D &offset, const Vector3D &target, double angle,
                   plane_t plane) {
  MachineAdapter::arc(offset, target, angle, plane);
  probePending = false;
}
