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

#include "PlannerCommand.h"

#include <gcode/Axes.h>


namespace GCode {
  class PlannerConfig;

  class LineCommand : public PlannerCommand {
  public:
    Axes target;
    double length;

    double entryVel;
    double exitVel;
    double deltaV;

    double maxVel;
    double maxAccel;
    double maxJerk;

    double times[7];

    Axes unit;

    bool seeking;

    LineCommand(uint64_t id, const Axes &start, const Axes &end, double feed,
                bool seeking, const PlannerConfig &config);

    // From PlannerCommand
    const char *getType() const {return "line";}
    bool isSeeking() const {return seeking;}
    bool isMove() const {return true;}

    double getEntryVelocity() const {return entryVel;}
    void setEntryVelocity(double entryVel) {this->entryVel = entryVel;}
    double getExitVelocity() const {return exitVel;}
    void setExitVelocity(double exitVel) {this->exitVel = exitVel;}
    double getDeltaVelocity() const {return deltaV;}
    double getLength() const {return length;}
    void restart(const Axes &position, const PlannerConfig &config);

    void insert(cb::JSON::Sink &sink) const;

  protected:
    void computeLimits(const Axes &start, const PlannerConfig &config);
  };
}
