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

#include "Settings.h"

#include <cbang/json/Writer.h>
#include <cbang/json/Reader.h>


using namespace std;
using namespace cb;
using namespace CAMotics;


bool Settings::has(const string &name) const {
  return contains(QString::fromUtf8(name.c_str()));
}


QVariant Settings::get(const string &name, const QVariant &defaultValue) const {
  return value(QString::fromUtf8(name.c_str()), defaultValue);
}


void Settings::set(const string &name, const QVariant &value) {
  setValue(QString::fromUtf8(name.c_str()), value);
}


Vector3D Settings::getVector3D(const string &name,
                               const Vector3D &defaultValue) const {
  if (!has(name)) return defaultValue;

  auto value = JSON::Reader::parseString(get(name).toString().toStdString());

  Vector3D v;
  v.read(*value);
  return v;
}


void Settings::setVector3D(const string &name, const Vector3D &value) {
  set(name, JSON::Writer::toString(value).c_str());
}
