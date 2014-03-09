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

#include "Pin.h"

#include <algorithm>

using namespace std;
using namespace OpenSCAM::PCB;


Pin::Pin() : thickness(0), clearance(0), mask(0), drill(0) {}


Pin::Pin(int drill, int pad, int clearance, int mask) :
  thickness(drill + pad), clearance(clearance), mask(drill + pad + mask),
  drill(drill) {
  if (pad == 0) flags = "hole";
}


void Pin::rotate(const Point &center, double angle) {
  p.rotate(center, angle);
}


void Pin::translate(const Point &t) {
  p += t;
}


void Pin::multiply(double m) {
  p.multiply(m);
  thickness *= m;
  clearance *= m;
  mask *= m;
  drill *= m;
}


void Pin::round(int i) {
  p.round(i);
  Object::round(thickness, i);
  Object::round(clearance, i);
  Object::round(mask, i);
  Object::round(drill, i);
}


void Pin::write(ostream &stream) const {
  stream << "Pin[" << p << " " << thickness << " " << clearance << " "
         << mask << " " << drill << " \"" << name << "\" \"" << number
         << "\" \"" << flags << "\"]";
}


void Pin::read(istream &stream) {
  stream >> p >> thickness >> clearance >> mask >> drill;
  name = readString(stream);
  number = readString(stream);
  flags = readString(stream);
}


void Pin::bounds(Point &min, Point &max) const {
  int x = std::max(thickness, std::max(mask, drill)) / 2;
  Point offset(x, x);

  Point(p - offset).bounds(min, max);
  Point(p + offset).bounds(min, max);
}


void Pin::flipX(double x) {
  p.flipX(x);
}


void Pin::flipY(double y) {
  p.flipY(y);
}
