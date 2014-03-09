/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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
using namespace tplang;

#define INCHES_PER_MM 0.0393701
#define MM_PER_INCH 25.4
#define FEET_PER_METER 3.28084
#define METER_PER_FOOT 0.3048


double MachineUnitAdapter::getFeed(feed_mode_t *_mode) const {
  feed_mode_t mode;
  double feed = MachineAdapter::getFeed(&mode);

  if (_mode) *_mode = mode;

  return mode == INVERSE_TIME || units == METRIC ? feed : feed * INCHES_PER_MM;
}


void MachineUnitAdapter::setFeed(double feed, feed_mode_t mode) {
  MachineAdapter::setFeed(mode == INVERSE_TIME || units == METRIC ? feed :
                 feed * MM_PER_INCH, mode);
}


double MachineUnitAdapter::getSpeed(spin_mode_t *_mode, double *max) const {
  spin_mode_t mode;
  double speed = MachineAdapter::getSpeed(&mode, max);

  if (_mode) *_mode = mode;

  return mode != CONSTANT_SURFACE_SPEED || units == METRIC ? speed :
    speed * FEET_PER_METER;
}


void MachineUnitAdapter::setSpeed(double speed, spin_mode_t mode, double max) {
  MachineAdapter::setSpeed(mode != CONSTANT_SURFACE_SPEED || units == METRIC ?
                           speed : speed * METER_PER_FOOT, mode, max);
}


Axes MachineUnitAdapter::getPosition() const {
  return units == METRIC ?
    MachineAdapter::getPosition() :
    MachineAdapter::getPosition() * INCHES_PER_MM;
}


Vector3D MachineUnitAdapter::getPosition(axes_t axes) const {
  return units == METRIC ?
    MachineAdapter::getPosition(axes) :
    MachineAdapter::getPosition(axes) * INCHES_PER_MM;
}


void MachineUnitAdapter::move(const Axes &axes, bool rapid) {
  MachineAdapter::move(axes * (units == METRIC ? 1 : MM_PER_INCH), rapid);
}


void MachineUnitAdapter::arc(const Vector3D &offset, double angle,
                             plane_t plane) {
  if (units == METRIC) MachineAdapter::arc(offset, angle, plane);
  else MachineAdapter::arc(offset * MM_PER_INCH, angle, plane);
}
