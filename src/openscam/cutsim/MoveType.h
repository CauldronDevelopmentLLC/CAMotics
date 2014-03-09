/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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
#ifndef CBANG_MOVE_TYPE_H
#define CBANG_MOVE_TYPE_H

#define CBANG_ENUM_NAME MoveType
#define CBANG_ENUM_NAMESPACE OpenSCAM
#define CBANG_ENUM_PATH openscam/cutsim
#define CBANG_ENUM_PREFIX 5
#include <cbang/enum/MakeEnumeration.def>

#endif // CBANG_MOVE_TYPE_H
#else // CBANG_ENUM_EXPAND

CBANG_ENUM_EXPAND(MOVE_RAPID,      0)
CBANG_ENUM_EXPAND(MOVE_CUTTING,    1)
CBANG_ENUM_EXPAND(MOVE_PROBE,      2)
CBANG_ENUM_EXPAND(MOVE_DRILL,      3)

#endif // CBANG_ENUM_EXPAND
