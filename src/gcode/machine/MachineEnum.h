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

#pragma once

#include "PortType.h"

#include <gcode/VarTypes.h>


namespace GCode {
  class MachineEnum : public PortType::Enum, public VarTypesEnumerationBase {
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

    static inline VarTypes::enum_t getVarType(char letter) {
      switch (letter) {
      case 'A': return VT_A;
      case 'B': return VT_B;
      case 'C': return VT_C;
      case 'D': return VT_D;
      case 'E': return VT_E;
      case 'F': return VT_F;
        // G
      case 'H': return VT_H;
      case 'I': return VT_I;
      case 'J': return VT_J;
      case 'K': return VT_K;
      case 'L': return VT_L;
        // M
        // N
        // O
      case 'P': return VT_P;
      case 'Q': return VT_Q;
      case 'R': return VT_R;
      case 'S': return VT_S;
      case 'T': return VT_T;
      case 'U': return VT_U;
      case 'V': return VT_V;
      case 'W': return VT_W;
      case 'X': return VT_X;
      case 'Y': return VT_Y;
      case 'Z': return VT_Z;
      default: THROW("Invalid variable name " << letter);
      }
    }
  };
}
