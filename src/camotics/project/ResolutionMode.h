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
#ifndef CAMOTICS_RESOLUTION_MODE_H
#define CAMOTICS_RESOLUTION_MODE_H

#define CBANG_ENUM_NAME ResolutionMode
#define CBANG_ENUM_NAMESPACE CAMotics
#define CBANG_ENUM_PATH camotics/project
#define CBANG_ENUM_PREFIX 11
#include <cbang/enum/MakeEnumeration.def>

#endif // CAMOTICS_RESOLUTION_MODE_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(RESOLUTION_MANUAL,    0)
CBANG_ENUM_EXPAND(RESOLUTION_LOW,       1)
CBANG_ENUM_EXPAND(RESOLUTION_MEDIUM,    2)
CBANG_ENUM_EXPAND(RESOLUTION_HIGH,      3)
CBANG_ENUM_EXPAND(RESOLUTION_VERY_HIGH, 4)

#endif // CBANG_ENUM_EXPAND
