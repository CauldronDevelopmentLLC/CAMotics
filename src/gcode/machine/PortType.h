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

#ifndef CBANG_ENUM_EXPAND
#ifndef GCODE_PORT_TYPE_H
#define GCODE_PORT_TYPE_H

#define CBANG_ENUM_NAME PortType
#define CBANG_ENUM_NAMESPACE GCode
#define CBANG_ENUM_PATH gcode/machine
#include <cbang/enum/MakeEnumeration.def>

#endif // GCODE_PORT_TYPE_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM(X_MIN)
CBANG_ENUM(X_MAX)
CBANG_ENUM(Y_MIN)
CBANG_ENUM(Y_MAX)
CBANG_ENUM(Z_MIN)
CBANG_ENUM(Z_MAX)
CBANG_ENUM(A_MIN)
CBANG_ENUM(A_MAX)
CBANG_ENUM(B_MIN)
CBANG_ENUM(B_MAX)
CBANG_ENUM(C_MIN)
CBANG_ENUM(C_MAX)
CBANG_ENUM(U_MIN)
CBANG_ENUM(U_MAX)
CBANG_ENUM(V_MIN)
CBANG_ENUM(V_MAX)
CBANG_ENUM(W_MIN)
CBANG_ENUM(W_MAX)
CBANG_ENUM(PROBE)
CBANG_ENUM(FLOOD)
CBANG_ENUM(MIST)
CBANG_ENUM(ANALOG_IN_0)
CBANG_ENUM(ANALOG_IN_1)
CBANG_ENUM(ANALOG_IN_2)
CBANG_ENUM(ANALOG_IN_3)
CBANG_ENUM(DIGITAL_IN_0)
CBANG_ENUM(DIGITAL_IN_1)
CBANG_ENUM(DIGITAL_IN_2)
CBANG_ENUM(DIGITAL_IN_3)
CBANG_ENUM(ANALOG_OUT_0)
CBANG_ENUM(ANALOG_OUT_1)
CBANG_ENUM(ANALOG_OUT_2)
CBANG_ENUM(ANALOG_OUT_3)
CBANG_ENUM(DIGITAL_OUT_0)
CBANG_ENUM(DIGITAL_OUT_1)
CBANG_ENUM(DIGITAL_OUT_2)
CBANG_ENUM(DIGITAL_OUT_3)

#endif // CBANG_ENUM_EXPAND
