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


namespace GCode {
  class MachineEnum {
  public:
    typedef enum {MM_PER_MINUTE, INVERSE_TIME, MM_PER_REVOLUTION} feed_mode_t;
    typedef enum {REVOLUTIONS_PER_MINUTE, CONSTANT_SURFACE_SPEED} spin_mode_t;
    typedef enum {XY, XZ, YZ, YV, UV, UW, VW} plane_t;
    typedef enum {XYZ, ABC, UVW, AXES_COUNT} axes_t;
    typedef enum {PROBE, X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX, A_MIN, A_MAX,
                  B_MIN, B_MAX, C_MIN, C_MAX, U_MIN, U_MAX, V_MIN, V_MAX, W_MIN,
                  W_MAX} port_t;
  };
}
