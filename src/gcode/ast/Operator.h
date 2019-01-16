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
#ifndef GCODE_OPERATOR_H
#define GCODE_OPERATOR_H

#define CBANG_ENUM_NAME Operator
#define CBANG_ENUM_NAMESPACE GCode
#define CBANG_ENUM_PATH gcode/ast
#include <cbang/enum/MakeEnumeration.def>

#endif // GCODE_OPERATOR_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(NO_OP,       0)
CBANG_ENUM_EXPAND(EXP_OP,      1)
CBANG_ENUM_EXPAND(MUL_OP,      2)
CBANG_ENUM_EXPAND(DIV_OP,      3)
CBANG_ENUM_EXPAND(MOD_OP,      4)
CBANG_ENUM_EXPAND(ADD_OP,      5)
CBANG_ENUM_EXPAND(SUB_OP,      6)
CBANG_ENUM_EXPAND(EQ_OP,       7)
CBANG_ENUM_EXPAND(NE_OP,       8)
CBANG_ENUM_EXPAND(GT_OP,       9)
CBANG_ENUM_EXPAND(GE_OP,       10)
CBANG_ENUM_EXPAND(LT_OP,       11)
CBANG_ENUM_EXPAND(LE_OP,       12)
CBANG_ENUM_EXPAND(AND_OP,      13)
CBANG_ENUM_EXPAND(OR_OP,       14)
CBANG_ENUM_EXPAND(XOR_OP,      15)

#endif // CBANG_ENUM_EXPAND
