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

#include "PlannerConfig.h"

#include <gcode/Codes.h>

#include <cbang/log/Logger.h>
#include <cbang/json/JSON.h>

#include <limits>
#include <cctype>

using namespace GCode;
using namespace cb;
using namespace std;


PlannerConfig::PlannerConfig() :
  maxVel(10000), maxAccel(200000), maxJerk(50000000), junctionDeviation(0.05),
  junctionAccel(100000), minJunctionLength(0.01),
  minSoftLimit(numeric_limits<double>::quiet_NaN()),
  maxSoftLimit(numeric_limits<double>::quiet_NaN()), minTravel(0.000001),
  maxArcError(0.01), maxLookahead(4096), minMoveSecs(0.02), maxMergeLength(2) {}


bool PlannerConfig::hasOverride(const Code &code) const {
  return overrides.find(code) != overrides.end();
}


const string &PlannerConfig::getOverride(const Code &code) const {
  const auto it = overrides.find(code);
  if (it != overrides.end()) return it->second;

  THROWS("Override code " << code << " not found");
}


void PlannerConfig::read(const JSON::Value &value) {
  defaultUnits = Units::parse(value.getString("default-units", "METRIC"));
  outputUnits = Units::parse(value.getString("output-units", "METRIC"));

  if (value.hasDict("max-vel")) maxVel.read(value.getDict("max-vel"));
  if (value.hasDict("max-accel")) maxAccel.read(value.getDict("max-accel"));
  if (value.hasDict("max-jerk")) maxJerk.read(value.getDict("max-jerk"));

  junctionDeviation = value.getNumber("junction-deviation", junctionDeviation);
  junctionAccel = value.getNumber("junction-accel", junctionAccel);
  minJunctionLength = value.getNumber("min-junction-length", minJunctionLength);

  if (value.hasDict("min-soft-limit"))
    minSoftLimit.read(value.getDict("min-soft-limit"));
  if (value.hasDict("max-soft-limit"))
    maxSoftLimit.read(value.getDict("max-soft-limit"));

  minTravel = value.getNumber("min-travel", minTravel);
  maxArcError = value.getNumber("max-arc-error", maxArcError);
  maxLookahead = value.getNumber("max-lookahead", maxLookahead);
  minMoveSecs = value.getNumber("min-move-seconds", minMoveSecs);
  maxMergeLength = value.getNumber("max-merge-length", maxMergeLength);

  programStart = value.getString("program-start", "");

  if (value.hasDict("overrides")) {
    const JSON::Dict &dict = value.getDict("overrides");
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
  sink.insert("min-junction-length", minJunctionLength);

  sink.beginInsert("min-soft-limit");
  minSoftLimit.write(sink);

  sink.beginInsert("max-soft-limit");
  maxSoftLimit.write(sink);

  sink.insert("min-travel", minTravel);
  sink.insert("max-arc-error", maxArcError);
  sink.insert("max-lookahead", maxLookahead);
  sink.insert("min-move-seconds", minMoveSecs);
  sink.insert("max-merge-length", maxMergeLength);

  if (!programStart.empty()) sink.insert("program-start", programStart);

  if (!overrides.empty()) {
    sink.insertDict("overrides");

    for (auto const &i : overrides)
      sink.insert(SSTR(i.first), i.second);

    sink.endDict();
  }

  sink.endDict();
}
