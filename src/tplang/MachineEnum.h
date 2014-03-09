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

#ifndef TPLANG_MACHINE_ENUM_H
#define TPLANG_MACHINE_ENUM_H

namespace tplang {
  class MachineEnum {
  public:
    typedef enum {MM_PER_MINUTE, INVERSE_TIME, MM_PER_REVOLUTION} feed_mode_t;
    typedef enum {REVOLUTIONS_PER_MINUTE, CONSTANT_SURFACE_SPEED} spin_mode_t;
    typedef enum {IMMEDIATE, START_ON_RISE, START_ON_FALL, START_WHEN_HIGH,
                  START_WHEN_LOW, STOP_ON_RISE, STOP_ON_FALL, STOP_WHEN_HIGH,
                  STOP_WHEN_LOW} input_mode_t;
    typedef enum {XY, XZ, YZ, YV, UV, UW, VW} plane_t;
    typedef enum {XYZ, ABC, UVW, AXES_COUNT} axes_t;
    typedef enum {OK, TIMEOUT, CONDITION, LIMIT} async_error_t;
    typedef enum {MIST_COOLANT, FLOOD_COOLANT, PROBE, ANALOG, DIGITAL} port_t;
  };
}

#endif // TPLANG_MACHINE_ENUM_H

