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


namespace GCode {
  typedef enum {
    // 0-30 Local parameters
    // 31-5000 Global parameters

#define PROBE_RESULT_ADDR(AXIS) ((address_t)(PROBE_RESULT_X + (AXIS)))

    // Probe results
    PROBE_RESULT_X = 5061,
    PROBE_RESULT_Y,
    PROBE_RESULT_Z,
    PROBE_RESULT_A,
    PROBE_RESULT_B,
    PROBE_RESULT_C,
    PROBE_RESULT_U,
    PROBE_RESULT_V,
    PROBE_RESULT_W,
    PROBE_SUCCESS = 5070,

#define PREDEFINED_ADDR(FIRST, AXIS)                                    \
    ((address_t)(((FIRST) ? PREDEFINED1_X : PREDEFINED2_X) + (AXIS)))

    // G28 home (absolute position)
    PREDEFINED1_X = 5161,
    PREDEFINED1_Y,
    PREDEFINED1_Z,
    PREDEFINED1_A,
    PREDEFINED1_B,
    PREDEFINED1_C,
    PREDEFINED1_U,
    PREDEFINED1_V,
    PREDEFINED1_W,

    // G30 home (absolute position)
    PREDEFINED2_X = 5181,
    PREDEFINED2_Y,
    PREDEFINED2_Z,
    PREDEFINED2_A,
    PREDEFINED2_B,
    PREDEFINED2_C,
    PREDEFINED2_U,
    PREDEFINED2_V,
    PREDEFINED2_W,

    // G52/G92 offsets
#define GLOBAL_OFFSET_ADDR(AXIS) ((address_t)(GLOBAL_X_OFFSET + (AXIS)))

    GLOBAL_OFFSETS_ENABLED = 5210,
    GLOBAL_X_OFFSET = 5211,
    GLOBAL_Y_OFFSET,
    GLOBAL_Z_OFFSET,
    GLOBAL_A_OFFSET,
    GLOBAL_B_OFFSET,
    GLOBAL_C_OFFSET,
    GLOBAL_U_OFFSET,
    GLOBAL_V_OFFSET,
    GLOBAL_W_OFFSET,

    // Coordinate systems
#define COORD_SYSTEM_WIDTH 20
#define COORD_SYSTEM_ROTATION_MEMBER 9
#define COORD_SYSTEM_ADDR(CS, MEMBER)                       \
    ((address_t)(CS1_X_OFFSET - COORD_SYSTEM_WIDTH +  \
                       (CS) * COORD_SYSTEM_WIDTH + (MEMBER)))

    CURRENT_COORD_SYSTEM = 5220,

    CS1_X_OFFSET = 5221,
    CS1_Y_OFFSET,
    CS1_Z_OFFSET,
    CS1_A_OFFSET,
    CS1_B_OFFSET,
    CS1_C_OFFSET,
    CS1_U_OFFSET,
    CS1_V_OFFSET,
    CS1_W_OFFSET,
    CS1_ROTATION,

    CS2_X_OFFSET = 5241,
    CS2_Y_OFFSET,
    CS2_Z_OFFSET,
    CS2_A_OFFSET,
    CS2_B_OFFSET,
    CS2_C_OFFSET,
    CS2_U_OFFSET,
    CS2_V_OFFSET,
    CS2_W_OFFSET,
    CS2_ROTATION,

    CS3_X_OFFSET = 5261,
    CS3_Y_OFFSET,
    CS3_Z_OFFSET,
    CS3_A_OFFSET,
    CS3_B_OFFSET,
    CS3_C_OFFSET,
    CS3_U_OFFSET,
    CS3_V_OFFSET,
    CS3_W_OFFSET,
    CS3_ROTATION,

    CS4_X_OFFSET = 5281,
    CS4_Y_OFFSET,
    CS4_Z_OFFSET,
    CS4_A_OFFSET,
    CS4_B_OFFSET,
    CS4_C_OFFSET,
    CS4_U_OFFSET,
    CS4_V_OFFSET,
    CS4_W_OFFSET,
    CS4_ROTATION,

    CS5_X_OFFSET = 5301,
    CS5_Y_OFFSET,
    CS5_Z_OFFSET,
    CS5_A_OFFSET,
    CS5_B_OFFSET,
    CS5_C_OFFSET,
    CS5_U_OFFSET,
    CS5_V_OFFSET,
    CS5_W_OFFSET,
    CS5_ROTATION,

    CS6_X_OFFSET = 5321,
    CS6_Y_OFFSET,
    CS6_Z_OFFSET,
    CS6_A_OFFSET,
    CS6_B_OFFSET,
    CS6_C_OFFSET,
    CS6_U_OFFSET,
    CS6_V_OFFSET,
    CS6_W_OFFSET,
    CS6_ROTATION,

    CS7_X_OFFSET = 5341,
    CS7_Y_OFFSET,
    CS7_Z_OFFSET,
    CS7_A_OFFSET,
    CS7_B_OFFSET,
    CS7_C_OFFSET,
    CS7_U_OFFSET,
    CS7_V_OFFSET,
    CS7_W_OFFSET,
    CS7_ROTATION,

    CS8_X_OFFSET = 5361,
    CS8_Y_OFFSET,
    CS8_Z_OFFSET,
    CS8_A_OFFSET,
    CS8_B_OFFSET,
    CS8_C_OFFSET,
    CS8_U_OFFSET,
    CS8_V_OFFSET,
    CS8_W_OFFSET,
    CS8_ROTATION,

    CS9_X_OFFSET = 5381,
    CS9_Y_OFFSET,
    CS9_Z_OFFSET,
    CS9_A_OFFSET,
    CS9_B_OFFSET,
    CS9_C_OFFSET,
    CS9_U_OFFSET,
    CS9_V_OFFSET,
    CS9_W_OFFSET,
    CS9_ROTATION,

    USER_INPUT = 5399, // M66 result check or wait for input

    // Read only
#define TOOL_OFFSET_ADDR(AXIS) ((address_t)(TOOL_X_OFFSET + (AXIS)))

    TOOL_NUMBER = 5400, // Active tool, not current T value
    TOOL_X_OFFSET,
    TOOL_Y_OFFSET,
    TOOL_Z_OFFSET,
    TOOL_A_OFFSET,
    TOOL_B_OFFSET,
    TOOL_C_OFFSET,
    TOOL_U_OFFSET,
    TOOL_V_OFFSET,
    TOOL_W_OFFSET,
    TOOL_DIAMETER,
    TOOL_FRONTANGLE,
    TOOL_BACKANGLE,
    TOOL_ORIENTATION,

    // Current relative coordinates including all offsets in selected units
#define CURRENT_COORD_ADDR(AXIS) ((address_t)(CURRENT_X + (AXIS)))

    CURRENT_X = 5420,
    CURRENT_Y,
    CURRENT_Z,
    CURRENT_A,
    CURRENT_B,
    CURRENT_C,
    CURRENT_U,
    CURRENT_V,
    CURRENT_W,

    DEBUG_ENABLE = 5599, // Enable/disable output of (DEBUG,) messages

    TOOL_CHANGER_FAULT = 5600,
    TOOL_CHANGER_CODE,

    MAX_ADDRESS = 5602, // Max address in EMC2 code
  } address_t;

  // TODO 5161-5390 should be persisted through machining center power cycle
}
