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

#include "Move.h"

#include <cbang/Math.h>
#include <cbang/log/Logger.h>

using namespace GCode;
using namespace cb;
using namespace std;


Move::Move(MoveType type, const Axes &start, const Axes &end, double startTime,
           int tool, double feed, double speed, unsigned line) :
  Segment3D(start.getXYZ(), end.getXYZ()), type(type),
  start(start), end(end), tool(tool), speed(speed), line(line),
  dist(start.distance(end)), startTime(startTime) {

  if (type != MoveType::MOVE_RAPID && !feed)
    THROW("Cutting move with zero feed");
  if (type == MoveType::MOVE_RAPID) feed = 10000; // TODO FIXME!!!!

  setFeed(feed); // Computes time too
}


void Move::setFeed(double feed) {
  this->feed = feed;
  time = feed ? dist / feed * 60 : 0; // in seconds
}


Vector3D Move::getPtAtTime(double time) const {
  if (getEndTime() <= time) return getEndPt();
  if (time <= getStartTime()) return getStartPt();

  double delta = time - getStartTime();
  return getStartPt() + (getEndPt() - getStartPt()) * delta / getTime();
}
