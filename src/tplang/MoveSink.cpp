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

#include "MoveSink.h"

using namespace cb;
using namespace OpenSCAM;
using namespace tplang;


MoveSink::MoveSink(MoveStream &stream, Options &options) :
  stream(stream), rapidFeed(1000) {

  options.pushCategory("Machine");
  options.addTarget("rapid-feed", rapidFeed,
                    "The feed rate of rapid moves in mm/min.");
  options.popCategory();
}


void MoveSink::reset() {
  MachineAdapter::reset();
  first = true;
  probePending = false;
  time = 0;
}


double MoveSink::input(unsigned port, input_mode_t mode, double timeout,
                       bool error) {
  if (mode != IMMEDIATE) probePending = true;
  return MachineAdapter::input(port, mode, timeout, error);
}


void MoveSink::move(const Axes &axes, bool rapid) {
  if (first) first = false; // Ignore first move

  else if (getPosition() != axes) {
    MoveType type = rapid ? Move::MOVE_RAPID :
      (probePending ? Move::MOVE_PROBE : Move::MOVE_CUTTING);

    Move move(type, getPosition(), axes, time, getTool(),
              rapid ? rapidFeed : getFeed(), getSpeed(),
              getLocation().getStart().getLine());

    time += move.getTime();

    stream.move(move);

    probePending = false;
  }

  MachineAdapter::move(axes, rapid);
}


void MoveSink::arc(const Vector3D &offset, double angle, plane_t plane) {
  MachineAdapter::arc(offset, angle, plane);
  probePending = false;
}
