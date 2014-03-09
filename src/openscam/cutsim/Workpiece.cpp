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

#include "Workpiece.h"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Workpiece::Workpiece(const Rectangle3R &r) :
  Rectangle3R(r), center(getCenter()) {
  Vector3R halfDim = getDimensions() / 2;
  halfDim2 = halfDim * halfDim;
}
