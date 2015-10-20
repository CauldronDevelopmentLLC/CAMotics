/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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
using namespace CAMotics;

#define INCHES_PER_MM (1.0 / 25.4)
#define MM_PER_INCH 25.4
#define FEET_PER_METER (1.0 / 0.3048)
#define METER_PER_FOOT 0.3048


double MachineUnitAdapter::getFeed(feed_mode_t *_mode) const {
  feed_mode_t mode;
  double feed = MachineAdapter::getFeed(&mode);

  if (_mode) *_mode = mode;

  return mode == INVERSE_TIME ? feed : feed * mmInchIn();
}


void MachineUnitAdapter::setFeed(double feed, feed_mode_t mode) {
  MachineAdapter::setFeed
    (mode == INVERSE_TIME ? feed : feed * mmInchOut(), mode);
}


double MachineUnitAdapter::getSpeed(spin_mode_t *_mode, double *max) const {
  spin_mode_t mode;
  double speed = MachineAdapter::getSpeed(&mode, max);

  if (_mode) *_mode = mode;

  return mode != CONSTANT_SURFACE_SPEED ? speed : speed * meterFootIn();
}


void MachineUnitAdapter::setSpeed(double speed, spin_mode_t mode, double max) {
  MachineAdapter::setSpeed
    (mode != CONSTANT_SURFACE_SPEED ? speed : speed * meterFootOut(), mode,
     max);
}


Axes MachineUnitAdapter::getPosition() const {
  return MachineAdapter::getPosition() * mmInchIn();
}


Vector3D MachineUnitAdapter::getPosition(axes_t axes) const {
  return MachineAdapter::getPosition(axes) * mmInchIn();
}


void MachineUnitAdapter::move(const Axes &axes, bool rapid) {
  MachineAdapter::move(axes * mmInchOut(), rapid);
}


void MachineUnitAdapter::arc(const Vector3D &offset, double angle,
                             plane_t plane) {
  MachineAdapter::arc(offset * mmInchOut(), angle, plane);
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
