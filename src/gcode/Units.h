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
#ifndef GCODE_UNITS_H
#define GCODE_UNITS_H

#define CBANG_ENUM_NAME Units
#define CBANG_ENUM_NAMESPACE GCode
#define CBANG_ENUM_PATH gcode
#include <cbang/enum/MakeEnumeration.def>

namespace GCode {
  static inline double convert(Units src, Units dst, double value) {
    if (src == Units::IMPERIAL && dst == Units::METRIC) return value * 25.4;
    if (src == Units::METRIC && dst == Units::IMPERIAL) return value / 25.4;
    return value;
  }
}


#endif // GCODE_UNITS_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(NO_UNITS, -1)
CBANG_ENUM_EXPAND(METRIC,    0)
CBANG_ENUM_EXPAND(IMPERIAL,  1)

CBANG_ENUM_ALIAS(MM,   METRIC)
CBANG_ENUM_ALIAS(INCH, IMPERIAL)

#endif // CBANG_ENUM_EXPAND
