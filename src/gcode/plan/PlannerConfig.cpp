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
  start(0.0), maxVelocity(0.0), maxJerk(0.0), junctionDeviation(0.05),
  junctionAccel(100000) {}


void PlannerConfig::read(const JSON::Value &value) {
  units = Units::parse(value.getString("units", "METRIC"));

  if (value.hasDict("start")) start.read(value.getDict("start"));
  else start = Axes(0.0);

  if (value.hasDict("max-velocity"))
    maxVelocity.read(value.getDict("max-velocity"));
  else maxVelocity = Axes(0.0);

  if (value.hasDict("max-jerk")) maxJerk.read(value.getDict("max-jerk"));
  else maxJerk = Axes(0.0);

  junctionDeviation = value.getNumber("junction-deviation", 0.05);
  junctionAccel = value.getNumber("junction-acceleration", 100000);
}


void PlannerConfig::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("units", units.toString());

  sink.beginInsert("start");
  start.write(sink);

  sink.beginInsert("max-velocity");
  maxVelocity.write(sink);

  sink.beginInsert("max-jerk");
  maxJerk.write(sink);

  sink.insert("junction-deviation", junctionDeviation);
  sink.insert("junction-acceleration", junctionAccel);

  sink.endDict();
}
