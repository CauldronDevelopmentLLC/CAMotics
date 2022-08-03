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
#include <gcode/machine/MachineEnum.h>

#include <cbang/json/Serializable.h>

#include <map>
#include <limits>


namespace GCode {
  class PlannerConfig :
    public cb::JSON::Serializable, public MachineEnum {
  public:
    Axes maxVel              = 10000;
    Axes maxAccel            = 200000;
    Axes maxJerk             = 50000000;

    double junctionDeviation = 0.05;
    double junctionAccel     = 200000;

    Axes minSoftLimit;
    Axes maxSoftLimit;

    Units defaultUnits;
    Units outputUnits;
    double minTravel         = 0.000001;
    double maxArcError       = 0.01;
    unsigned maxLookahead    = 4096;
    path_mode_t pathMode     = CONTINUOUS_MODE;
    double maxBlendError     = 0.1;   // mm
    double minMergeError     = 0.001; // mm
    double maxMergeError     = 0.1;   // mm
    double maxMergeLength    = 10;    // mm
    bool rapidAutoOff        = false;
    unsigned idBits          = 16;

    std::string programStart;
    std::map<Code, std::string> overrides;

    PlannerConfig() {}

    bool hasOverride(const Code &code) const;
    const std::string &getOverride(const Code &code) const;

    bool softLimitValid(int axis) const;

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
