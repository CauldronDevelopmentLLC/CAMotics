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
#ifndef CAMOTICS_UNITS_H
#define CAMOTICS_UNITS_H

#define CBANG_ENUM_NAME Units
#define CBANG_ENUM_NAMESPACE CAMotics
#define CBANG_ENUM_PATH camotics
#define CBANG_ENUM_PREFIX 0
#include <cbang/enum/MakeEnumeration.def>

#endif // CAMOTICS_UNITS_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(IMPERIAL, 0)
CBANG_ENUM_EXPAND(METRIC,   1)

#endif // CBANG_ENUM_EXPAND
