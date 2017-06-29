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

#include <gcode/Axes.h>


namespace GCode {
  class PlannerSink {
  public:
    virtual ~PlannerSink() {}

    virtual void start() = 0;
    virtual void end() = 0;

    virtual void setSpeed(double speed) = 0;
    virtual void setTool(unsigned tool) = 0;

    virtual void dwell(double seconds) = 0;
    virtual void pause(bool optional) = 0;

    virtual void move(const Axes &position, double maxVel) = 0;
  };
}
