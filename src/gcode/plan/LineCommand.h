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

#include "PlannerCommand.h"

#include <gcode/Axes.h>


namespace GCode {
  class PlannerConfig;

  class LineCommand : public PlannerCommand {
  public:
    double feed;

    Axes start;
    Axes target;
    double length   = 0;

    double entryVel = 0;
    double exitVel  = 0;
    double deltaV   = 0;

    double maxVel   = 0;
    double maxAccel = 0;
    double maxJerk  = 0;

    double targetJunctionVel = 0;

    double times[7] = {0};

    struct Speed {
      double offset;
      double speed;
      Speed(double offset, double speed) : offset(offset), speed(speed) {}
    };

    std::vector<Axes> merged;
    std::vector<Speed> speeds;

    Axes unit;

    bool rapid;
    bool seeking;
    bool first;

    LineCommand(const Axes &start, const Axes &end, double feed, bool rapid,
                bool seeking, bool first, const PlannerConfig &config);

    bool canBlend() const;
    bool canMerge() const;

    // From PlannerCommand
    const char *getType() const {return "line";}
    bool isRapid() const {return rapid;}
    bool isSeeking() const {return seeking;}
    bool isMove() const {return true;}

    double getEntryVelocity() const {return entryVel;}
    void setEntryVelocity(double entryVel) {this->entryVel = entryVel;}
    double getExitVelocity() const {return exitVel;}
    void setExitVelocity(double exitVel) {this->exitVel = exitVel;}
    double getDeltaVelocity() const {return deltaV;}
    double getLength() const {return length;}
    double getTime() const;

    bool merge(const LineCommand &lc, const PlannerConfig &config,
               double speed);
    void restart(const Axes &position, const PlannerConfig &config);
    void insert(cb::JSON::Sink &sink) const;
    void write(MachineInterface &machine) const;
    void cut(double length, std::vector<Speed> &speeds, double offset = 0,
             bool fromEnd = true);

  protected:
    void computeLimits(const PlannerConfig &config);
  };
}
