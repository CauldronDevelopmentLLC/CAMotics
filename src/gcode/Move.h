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

#pragma once


#include "MoveType.h"

#include <gcode/Tool.h>
#include <gcode/Axes.h>

#include <cbang/geom/Segment.h>

#include <ostream>


namespace GCode {
  class Move : protected cb::Segment3D, public MoveType {
  protected:
    MoveType type;
    Axes start;
    Axes end;
    int tool;
    double feed;
    double speed;
    unsigned line;

    double dist;
    double time;
    double startTime;

  public:
    Move() : type(MoveType::MOVE_RAPID), tool(0), feed(0), speed(0), line(0),
             dist(0), time(0), startTime(0) {}
    Move(MoveType type, const Axes &start, const Axes &end,
         double startTime, int tool, double feed, double speed, unsigned line);

    MoveType getType() const {return type;}
    const Axes &getStart() const {return start;}
    const Axes &getEnd() const {return end;}
    const cb::Vector3D &getStartPt() const {return cb::Segment3D::getStart();}
    const cb::Vector3D &getEndPt() const {return cb::Segment3D::getEnd();}
    int getTool() const {return tool;}
    double getFeed() const {return feed;}
    void setFeed(double feed);
    double getSpeed() const {return speed;}
    unsigned getLine() const {return line;}

    double getDistance() const {return dist;}
    double getTime() const {return time;}
    double getStartTime() const {return startTime;}
    double getEndTime() const {return startTime + time;}

    cb::Vector3D getPtAtTime(double time) const;

    using cb::Segment3D::reverse;
    using cb::Segment3D::distance;
  };
}
