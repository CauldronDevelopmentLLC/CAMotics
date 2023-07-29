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

#include "Color.h"

using namespace CAMotics;


unsigned Color::toIndex(float r, float g, float b) {
  return
    ((unsigned)(r * 0xff) <<  0) +
    ((unsigned)(g * 0xff) <<  8) +
    ((unsigned)(b * 0xff) << 16) - 1;
}


Color Color::fromIndex(unsigned i) {
  i++;

  float r = ((i >>  0) & 0xff) / (float)0xff;
  float g = ((i >>  8) & 0xff) / (float)0xff;
  float b = ((i >> 16) & 0xff) / (float)0xff;

  return Color(r, g, b);
}

const Color Color::RED(1, 0, 0);
const Color Color::GREEN(0, 1, 0);
const Color Color::BLUE(0, 0, 1);
const Color Color::CYAN(0, 1, 1);
const Color Color::YELLOW(1, 1, 0);
const Color Color::PURPLE(1, 0, 1);
const Color Color::WHITE(1, 1, 1);
