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

#include "MachineEnum.h"

using namespace GCode;


VarTypes::enum_t MachineEnum::getVarType(char letter) {
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


const char *MachineEnum::toString(path_mode_t mode) {
  return PathMode(mode).toString();
}


const char *MachineEnum::toString(feed_mode_t mode) {
  switch (mode) {
  case INVERSE_TIME:         return "INVERSE_TIME";
  case UNITS_PER_MINUTE:     return "UNITS_PER_MINUTE";
  case UNITS_PER_REVOLUTION: return "UNITS_PER_REVOLUTION";
  }

  THROW("Invalid feed mode " << mode);
}


const char *MachineEnum::toString(spin_mode_t mode) {
  switch (mode) {
  case REVOLUTIONS_PER_MINUTE: return "REVOLUTIONS_PER_MINUTE";
  case CONSTANT_SURFACE_SPEED: return "CONSTANT_SURFACE_SPEED";
  }

  THROW("Invalid spin mode " << mode);
}


const char *MachineEnum::toString(input_mode_t mode) {
  switch (mode) {
  case INPUT_IMMEDIATE: return "IMMEDIATE";
  case INPUT_RISE:      return "RISE";
  case INPUT_FALL:      return "FALL";
  case INPUT_HIGH:      return "HIGH";
  case INPUT_LOW:       return "LOW";
  }

  THROW("Invalid input mode " << mode);
}


const char *MachineEnum::toString(plane_t plane) {
  switch (plane) {
  case XY: return "XY";
  case UV: return "UV";
  case XZ: return "XZ";
  case UW: return "UW";
  case YZ: return "YZ";
  case VW: return "VW";
  }

  THROW("Invalid plane " << plane);
}


const char *MachineEnum::toString(port_t port) {
  return PortType(port).toString();
}
