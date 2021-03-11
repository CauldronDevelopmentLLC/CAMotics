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

#include "MachineUnitAdapter.h"

using namespace cb;
using namespace GCode;

#define INCHES_PER_MM (1.0 / 25.4)
#define MM_PER_INCH 25.4
#define FEET_PER_METER (1.0 / 0.3048)
#define METER_PER_FOOT 0.3048


double MachineUnitAdapter::getFeed() const {
  double feed = MachineAdapter::getFeed();
  return getFeedMode() == INVERSE_TIME ? feed : feed * mmInchIn();
}


void MachineUnitAdapter::setFeed(double feed) {
  MachineAdapter::setFeed
    (getFeedMode() == INVERSE_TIME ? feed : feed * mmInchOut());
}


double MachineUnitAdapter::getSpeed() const {
  double speed = MachineAdapter::getSpeed();
  return
    getSpinMode() != CONSTANT_SURFACE_SPEED ? speed : speed * meterFootIn();
}


void MachineUnitAdapter::setSpeed(double speed) {
  MachineAdapter::setSpeed
    (getSpinMode() != CONSTANT_SURFACE_SPEED ? speed : speed * meterFootOut());
}


void MachineUnitAdapter::setPathMode(path_mode_t mode, double motionBlending,
                                     double naiveCAM) {
  MachineAdapter::setPathMode(mode, motionBlending * mmInchOut(),
                              naiveCAM * mmInchOut());
}


Axes MachineUnitAdapter::getPosition() const {
  return MachineAdapter::getPosition() * mmInchIn();
}


Vector3D MachineUnitAdapter::getPosition(axes_t axes) const {
  return MachineAdapter::getPosition(axes) * mmInchIn();
}


void MachineUnitAdapter::setPosition(const Axes &position) {
  MachineAdapter::setPosition(position * mmInchOut());
}


void MachineUnitAdapter::move(const Axes &position, int axes, bool rapid,
                              double time) {
  MachineAdapter::move(position * mmInchOut(), axes, rapid, time);
}


void MachineUnitAdapter::arc(const Vector3D &offset, const Vector3D &target,
                             double angle, plane_t plane) {
  MachineAdapter::arc(offset * mmInchOut(), target * mmInchOut(), angle, plane);
}


double MachineUnitAdapter::mmInchIn() const {
  return units == targetUnits ? 1 :
    (targetUnits == METRIC ? INCHES_PER_MM : MM_PER_INCH);
}


double MachineUnitAdapter::mmInchOut() const {
  return units == targetUnits ? 1 :
    (targetUnits == METRIC ? MM_PER_INCH : INCHES_PER_MM);
}


double MachineUnitAdapter::meterFootIn() const {
  return units == targetUnits ? 1 :
    (targetUnits == METRIC ? FEET_PER_METER : METER_PER_FOOT);
}


double MachineUnitAdapter::meterFootOut() const {
  return units == targetUnits ? 1 :
    (targetUnits == METRIC ? METER_PER_FOOT : FEET_PER_METER);
}
