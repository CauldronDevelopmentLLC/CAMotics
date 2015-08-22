/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_BOUNDS_VIEW_H
#define CAMOTICS_BOUNDS_VIEW_H

#include <camotics/Geom.h>

namespace CAMotics {
  class BoundsView : Rectangle3R {
  public:
    BoundsView(const Rectangle3R &r) : Rectangle3R(r) {}
    BoundsView(const Vector3R &p1, const Vector3R &p2) : Rectangle3R(p1, p2) {}

    void draw();
  };
}

#endif // CAMOTICS_BOUNDS_VIEW_H

