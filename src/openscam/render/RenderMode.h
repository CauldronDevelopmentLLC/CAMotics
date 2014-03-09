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
#ifndef OPENSCAM_RENDER_MODE_H
#define OPENSCAM_RENDER_MODE_H

#define CBANG_ENUM_NAME RenderMode
#define CBANG_ENUM_NAMESPACE OpenSCAM
#define CBANG_ENUM_PATH openscam/render
#include <cbang/enum/MakeEnumeration.def>

#endif // OPENSCAM_RENDER_MODE_H
#else // CBANG_ENUM_EXPAND

// EDX Register
CBANG_ENUM_EXPAND(MCUBES_MODE,  1)
CBANG_ENUM_EXPAND(MTETRA_MODE,  2)
CBANG_ENUM_EXPAND(DC_MODE,      3)
CBANG_ENUM_EXPAND(HERMITE_MODE, 4)

#endif // CBANG_ENUM_EXPAND
