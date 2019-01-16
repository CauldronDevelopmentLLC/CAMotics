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

#ifndef CBANG_ENUM_EXPAND
#ifndef GCODE_VAR_TYPES_H
#define GCODE_VAR_TYPES_H

#define CBANG_ENUM_NAME VarTypes
#define CBANG_ENUM_NAMESPACE GCode
#define CBANG_ENUM_PATH gcode
#define CBANG_ENUM_PREFIX 3
#include <cbang/enum/MakeEnumeration.def>

#endif // GCODE_VAR_TYPES_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(VT_NONE,   0)
CBANG_ENUM_EXPAND(VT_A,      1 << 0)
CBANG_ENUM_EXPAND(VT_B,      1 << 1)
CBANG_ENUM_EXPAND(VT_C,      1 << 2)
CBANG_ENUM_EXPAND(VT_D,      1 << 3)
CBANG_ENUM_EXPAND(VT_E,      1 << 4)
CBANG_ENUM_EXPAND(VT_F,      1 << 5)
// G
CBANG_ENUM_EXPAND(VT_H,      1 << 7)
CBANG_ENUM_EXPAND(VT_I,      1 << 8)
CBANG_ENUM_EXPAND(VT_J,      1 << 9)
CBANG_ENUM_EXPAND(VT_K,      1 << 10)
CBANG_ENUM_EXPAND(VT_L,      1 << 11)
// M
// N
// O
CBANG_ENUM_EXPAND(VT_P,      1 << 15)
CBANG_ENUM_EXPAND(VT_Q,      1 << 16)
CBANG_ENUM_EXPAND(VT_R,      1 << 17)
CBANG_ENUM_EXPAND(VT_S,      1 << 18)
CBANG_ENUM_EXPAND(VT_T,      1 << 19)
CBANG_ENUM_EXPAND(VT_U,      1 << 20)
CBANG_ENUM_EXPAND(VT_V,      1 << 21)
CBANG_ENUM_EXPAND(VT_W,      1 << 22)
CBANG_ENUM_EXPAND(VT_X,      1 << 23)
CBANG_ENUM_EXPAND(VT_Y,      1 << 24)
CBANG_ENUM_EXPAND(VT_Z,      1 << 25)

CBANG_ENUM_EXPAND(VT_XYZ,    VT_X | VT_Y | VT_Z)
CBANG_ENUM_EXPAND(VT_ABC,    VT_A | VT_B | VT_C)
CBANG_ENUM_EXPAND(VT_UVW,    VT_U | VT_V | VT_W)
CBANG_ENUM_EXPAND(VT_IJK,    VT_I | VT_J | VT_K)
CBANG_ENUM_EXPAND(VT_RLQ,    VT_R | VT_L | VT_Q)

CBANG_ENUM_EXPAND(VT_AXIS,   VT_XYZ | VT_ABC | VT_UVW)
CBANG_ENUM_EXPAND(VT_ANGLE,  VT_IJK)
CBANG_ENUM_EXPAND(VT_CANNED, VT_XYZ | VT_UVW | VT_R | VT_L)

#endif // CBANG_ENUM_EXPAND
