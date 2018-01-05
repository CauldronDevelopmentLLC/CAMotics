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

#include <cbang/geom/Vector.h>


namespace GCode {
  class PlannerConfig;

  class LineCommand : public PlannerCommand {
  public:
    cb::Vector4D target;
    double length;

    double entryVel;
    double exitVel;
    double deltaV;

    double maxVel;
    double maxAccel;
    double maxJerk;

    double times[7];

    cb::Vector4D unit;

    LineCommand(uint64_t id, const cb::Vector4D &start,
                const cb::Vector4D &end, double feed,
                const PlannerConfig &config);

    // From PlannerCommand
    const char *getType() const {return "line";}

    double getEntryVelocity() const {return entryVel;}
    void setEntryVelocity(double entryVel) {this->entryVel = entryVel;}
    double getExitVelocity() const {return exitVel;}
    void setExitVelocity(double exitVel) {this->exitVel = exitVel;}
    double getDeltaVelocity() const {return deltaV;}
    double getLength() const {return length;}
    void restart(const Axes &position, const PlannerConfig &config);

    void insert(cb::JSON::Sink &sink) const;

  protected:
    void computeLimits(const cb::Vector4D &start, const PlannerConfig &config);
  };
}
