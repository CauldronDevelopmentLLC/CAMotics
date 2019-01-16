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

#include <gcode/Axes.h>
#include <gcode/Units.h>
#include <gcode/Codes.h>

#include <cbang/json/Serializable.h>

#include <map>


namespace GCode {
  class PlannerConfig : public cb::JSON::Serializable {
  public:
    Axes maxVel;
    Axes maxAccel;
    Axes maxJerk;

    double junctionDeviation;
    double junctionAccel;
    double minJunctionLength;

    Axes minSoftLimit;
    Axes maxSoftLimit;

    Units defaultUnits;
    Units outputUnits;
    double minTravel;
    double maxArcError;
    unsigned maxLookahead;
    double minMoveSecs;
    double maxMergeLength;
    double maxMergeError;
    double maxCollinearAngle;
    bool rapidAutoOff;
    unsigned idBits;

    std::string programStart;
    std::map<const Code, std::string> overrides;

    PlannerConfig();

    bool hasOverride(const Code &code) const;
    const std::string &getOverride(const Code &code) const;

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
