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

#include <cbang/json/JSON.h>

#include <limits>

using namespace GCode;
using namespace cb;
using namespace std;


PlannerConfig::PlannerConfig() :
  start(0.0), maxVel(10000), maxAccel(200000), maxJerk(50000000),
  junctionDeviation(0.05), junctionAccel(100000), minTravel(0.000001),
  maxArcError(0.01), maxLookahead(4096) {}


void PlannerConfig::read(const JSON::Value &value) {
  defaultUnits = Units::parse(value.getString("default-units", "METRIC"));
  outputUnits = Units::parse(value.getString("output-units", "METRIC"));

  if (value.hasDict("start")) start.read(value.getDict("start"));
  if (value.hasDict("max-vel")) maxVel.read(value.getDict("max-vel"));
  if (value.hasDict("max-accel")) maxAccel.read(value.getDict("max-accel"));
  if (value.hasDict("max-jerk")) maxJerk.read(value.getDict("max-jerk"));

  junctionDeviation = value.getNumber("junction-deviation", junctionDeviation);
  junctionAccel = value.getNumber("junction-accel", junctionAccel);
  minTravel = value.getNumber("min-travel", minTravel);
  maxArcError = value.getNumber("max-arc-error", maxArcError);
  maxLookahead = value.getNumber("max-lookahead", maxLookahead);
}


void PlannerConfig::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("default-units", defaultUnits.toString());
  sink.insert("output-units", outputUnits.toString());

  sink.beginInsert("start");
  start.write(sink);

  sink.beginInsert("max-vel");
  maxVel.write(sink);

  sink.beginInsert("max-accel");
  maxAccel.write(sink);

  sink.beginInsert("max-jerk");
  maxJerk.write(sink);

  sink.insert("junction-deviation", junctionDeviation);
  sink.insert("junction-accel", junctionAccel);
  sink.insert("min-travel", minTravel);
  sink.insert("max-arc-error", maxArcError);
  sink.insert("max-lookahead", maxLookahead);

  sink.endDict();
}
