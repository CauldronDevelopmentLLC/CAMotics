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

#pragma once

#include "PortType.h"


namespace GCode {
  class MachineEnum : public PortType::Enum {
  public:
    typedef enum {
      DIR_OFF,
      DIR_CLOCKWISE,
      DIR_COUNTERCLOCKWISE,
    } dir_t;


    typedef enum {
      EXACT_PATH_MODE,
      EXACT_STOP_MODE,
      CONTINUOUS_MODE,
    } path_mode_t;


    typedef enum {
      RETURN_TO_R,
      RETURN_TO_OLD_Z,
    } return_mode_t;


    typedef enum {
      UNITS_PER_MINUTE,
      INVERSE_TIME,
      UNITS_PER_REVOLUTION
    } feed_mode_t;


    typedef enum {
      REVOLUTIONS_PER_MINUTE,
      CONSTANT_SURFACE_SPEED
    } spin_mode_t;


    typedef enum {
      PAUSE_PROGRAM,
      PAUSE_OPTIONAL,
      PAUSE_PALLET_CHANGE,
    } pause_t;

    typedef enum {
      XY = 170,
      UV = 171,
      XZ = 180,
      UW = 181,
      YZ = 190,
      VW = 191,
    } plane_t;


    typedef enum {
      XYZ,
      ABC,
      UVW,
      AXES_COUNT
    } axes_t;


    typedef enum {
      INPUT_IMMEDIATE,
      INPUT_RISE,
      INPUT_FALL,
      INPUT_HIGH,
      INPUT_LOW,
    } input_mode_t;


    typedef PortType::enum_t port_t;
  };
}
