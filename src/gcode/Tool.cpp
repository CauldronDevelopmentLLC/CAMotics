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

#include "Tool.h"

#include <cbang/String.h>
#include <cbang/Math.h>
#include <cbang/Exception.h>
#include <cbang/xml/XMLWriter.h>
#include <cbang/Catch.h>

#include <string.h>


using namespace std;
using namespace cb;
using namespace GCode;


const char *Tool::VARS = "XYZABCUVWRIJQ";
Tool Tool::null;


Tool::Tool(unsigned number, unsigned pocket, Units units) :
  number(number), pocket(pocket), units(units),
  shape(ToolShape::TS_CYLINDRICAL), snubDiameter(0) {

  Axes::clear();
  memset(vars, 0, sizeof(vars));

  if (units == Units::METRIC) {
    setLength(10);
    setRadius(1);

  } else {
    setLength(25.4);
    setRadius(25.4 / 16);
  }
}


string Tool::getSizeText() const {
  double diameter = getDiameter();
  double length = getLength();

  if (getUnits() == Units::IMPERIAL) {
    diameter /= 25.4;
    length /= 25.4;
  }

  string s;
  if (getShape() == ToolShape::TS_CONICAL)
    s = String::printf("%gdeg %g", getAngle(), diameter);
  else s = String::printf("%gx%g", diameter, length);

  if (getUnits() == Units::METRIC) s += "mm";
  else s += "in";

  return s;
}


string Tool::getText() const {
  if (!description.empty()) return description;

  return getSizeText() + " " +
    String::capitalize(String::toLower(getShape().toString()));
}


double Tool::getAngle() const {
  double angle = 180.0 - 360.0 * atan(getLength() / getRadius()) / M_PI;
  return round(angle * 100) / 100;
}


void Tool::setRadiusFromAngle(double angle) {
  setRadius(getLength() / tan((1 - angle / 180.0) * M_PI / 2.0));
}


void Tool::setLengthFromAngle(double angle) {
  setLength(getRadius() * tan((1 - angle / 180.0) * M_PI / 2.0));
}


ostream &Tool::print(ostream &stream) const {
  stream << "T" << number << " R" << getRadius() << " L" << getLength();
  return Axes::print(stream);
}


void Tool::write(JSON::Sink &sink, bool withNumber) const {
  sink.beginDict();

  double scale = units == Units::IMPERIAL ? 1.0 / 25.4 : 1;

  if (withNumber) sink.insert("number", number);
  sink.insert("units", String::toLower(getUnits().toString()));
  sink.insert("shape", String::toLower(getShape().toString()));
  sink.insert("length", getLength() * scale);
  sink.insert("diameter", getDiameter() * scale);
  if (getShape() == ToolShape::TS_SNUBNOSE)
    sink.insert("snub_diameter", getSnubDiameter() * scale);
  sink.insert("description", getDescription());

  sink.endDict();
}


void Tool::read(const JSON::Value &value) {
  setNumber(value.getU32("number", number));

  if (value.hasString("units"))
    units = Units::parse(value.getString("units"));

  double scale = units == Units::IMPERIAL ? 25.4 : 1;

  if (value.hasString("shape"))
    shape = ToolShape::parse(value.getString("shape"));

  if (value.hasNumber("length"))
    setLength(value.getNumber("length") * scale);

  if (value.hasNumber("diameter"))
    setDiameter(value.getNumber("diameter") * scale);

  if (value.hasNumber("snub_diameter"))
    setSnubDiameter(value.getNumber("snub_diameter") * scale);

  setDescription(value.getString("description", ""));
}
