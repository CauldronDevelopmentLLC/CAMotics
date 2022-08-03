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

#include "PlannerConfig.h"

#include <gcode/Codes.h>

#include <cbang/log/Logger.h>
#include <cbang/json/JSON.h>

#include <cctype>

using namespace GCode;
using namespace cb;
using namespace std;


bool PlannerConfig::hasOverride(const Code &code) const {
  return overrides.find(code) != overrides.end();
}


const string &PlannerConfig::getOverride(const Code &code) const {
  const auto it = overrides.find(code);
  if (it != overrides.end()) return it->second;

  THROW("Override code " << code << " not found");
}


bool PlannerConfig::softLimitValid(int axis) const {
  return minSoftLimit[axis] < maxSoftLimit[axis];
}


void PlannerConfig::read(const JSON::Value &value) {
  defaultUnits = Units::parse(value.getString("default-units", "METRIC"));
  outputUnits = Units::parse(value.getString("output-units", "METRIC"));

  if (value.hasDict("max-vel")) maxVel.read(value.getDict("max-vel"));
  if (value.hasDict("max-accel")) maxAccel.read(value.getDict("max-accel"));
  if (value.hasDict("max-jerk")) maxJerk.read(value.getDict("max-jerk"));

  junctionDeviation = value.getNumber("junction-deviation", junctionDeviation);
  junctionAccel = value.getNumber("junction-accel", junctionAccel);

  if (value.hasDict("min-soft-limit"))
    minSoftLimit.read(value.getDict("min-soft-limit"));
  if (value.hasDict("max-soft-limit"))
    maxSoftLimit.read(value.getDict("max-soft-limit"));

  minTravel = value.getNumber("min-travel", minTravel);
  maxArcError = value.getNumber("max-arc-error", maxArcError);
  maxLookahead = value.getNumber("max-lookahead", maxLookahead);
  if (value.hasString("path-mode"))
    pathMode = PathMode::parse(value.getString("path-mode"));
  maxBlendError = value.getNumber("max-blend-error", maxBlendError);
  minMergeError = value.getNumber("min-merge-error", minMergeError);
  maxMergeError = value.getNumber("max-merge-error", maxMergeError);
  maxMergeLength = value.getNumber("max-merge-length", maxMergeLength);
  rapidAutoOff = value.getBoolean("rapid-auto-off", rapidAutoOff);
  idBits = value.getU32("id-bits", idBits);

  if (idBits < 8 || 63 < idBits)
    THROW("'id-bits' cannot be less than 8 or greater than 63");

  programStart = value.getString("program-start", "");

  if (value.hasDict("overrides")) {
    auto &dict = value.getDict("overrides");
    for (unsigned i = 0; i < dict.size(); i++)
      overrides[Code::parse(dict.keyAt(i))] = dict.getString(i);
  }
}


void PlannerConfig::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("default-units", defaultUnits.toString());
  sink.insert("output-units", outputUnits.toString());

  sink.beginInsert("max-vel");
  maxVel.write(sink);

  sink.beginInsert("max-accel");
  maxAccel.write(sink);

  sink.beginInsert("max-jerk");
  maxJerk.write(sink);

  sink.insert("junction-deviation", junctionDeviation);
  sink.insert("junction-accel", junctionAccel);

  sink.beginInsert("min-soft-limit");
  minSoftLimit.write(sink);

  sink.beginInsert("max-soft-limit");
  maxSoftLimit.write(sink);

  sink.insert("min-travel", minTravel);
  sink.insert("max-arc-error", maxArcError);
  sink.insert("max-lookahead", maxLookahead);
  sink.insert("path-mode", PathMode(pathMode).toString());
  sink.insert("max-blend-error", maxBlendError);
  sink.insert("min-merge-error", minMergeError);
  sink.insert("max-merge-error", maxMergeError);
  sink.insert("max-merge-length", maxMergeLength);
  sink.insertBoolean("rapid-auto-off", rapidAutoOff);
  sink.insert("id-bits", idBits);

  if (!programStart.empty()) sink.insert("program-start", programStart);

  if (!overrides.empty()) {
    sink.insertDict("overrides");

    for (auto const &i : overrides)
      sink.insert(SSTR(i.first), i.second);

    sink.endDict();
  }

  sink.endDict();
}
