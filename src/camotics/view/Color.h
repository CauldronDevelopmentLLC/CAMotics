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

#include <cbang/geom/Vector.h>


namespace CAMotics {
  class Color : public cb::Vector4F {
    typedef cb::Vector<4, float> Super_T;

  public:
    Color(float r = 0, float g = 0, float b = 0, float a = 1) :
      cb::Vector<4, float>(r, g, b, a) {}
    Color(const Super_T &o) : Super_T(o) {}

    void setRed(double r)   {(*this)[0] = r;}
    void setGreen(double g) {(*this)[1] = g;}
    void setBlue(double b)  {(*this)[2] = b;}
    void setAlpha(double a) {(*this)[3] = a;}

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color CYAN;
    static const Color YELLOW;
    static const Color PURPLE;
 };
}
