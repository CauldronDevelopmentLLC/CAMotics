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

#include "Pad.h"

#include <algorithm>

using namespace std;
using namespace OpenSCAM::PCB;


Pad::Pad() : thickness(0), clearance(1000), mask(0) {}


Pad::Pad(unsigned int length, unsigned int width, int clearance, int mask) :
  thickness(0), clearance(clearance), mask(mask) {
  if (length < width) {
    thickness = length;

    p1.x() = thickness / 2;
    p1.y() = thickness / 2;

    p2.x() = p1.x();
    p2.y() = (int)width - thickness / 2;

  } else {
    thickness = width;

    p1.x() = thickness / 2;
    p1.y() = thickness / 2;

    p2.x() = length - thickness / 2;
    p2.y() = p1.y();
  }

  this->thickness = thickness;
  this->mask += thickness;
  flags = "square";
}


void Pad::rotate(const Point &center, double angle) {
  p1.rotate(center, angle);
  p2.rotate(center, angle);
}


void Pad::translate(const Point &t) {
  p1 += t;
  p2 += t;
}


void Pad::multiply(double m) {
  p1.multiply(m);
  p2.multiply(m);
  thickness *= m;
  clearance *= m;
  mask *= m;
}


void Pad::round(int i) {
  p1.round(i);
  p2.round(i);
  Object::round(thickness, i);
  Object::round(clearance, i);
  Object::round(mask, i);
}


void Pad::write(ostream &stream) const {
  stream << "Pad[" << p1 << " " << p2 << " " << thickness << " "
         << clearance << " " << mask << " \"" << name << "\" \"" << number
         << "\" \"" << flags << "\"]";
}


void Pad::read(istream &stream) {
  stream >> p1 >> p2 >> thickness >> clearance >> mask;
  name = readString(stream);
  number = readString(stream);
  flags = readString(stream);
}


void Pad::bounds(Point &min, Point &max) const {
  int x = std::max(thickness, mask) / 2;
  Point offset(x, x);

  Point(p1 - offset).bounds(min, max);
  Point(p1 + offset).bounds(min, max);
  Point(p2 - offset).bounds(min, max);
  Point(p2 + offset).bounds(min, max);
}


void Pad::flipX(double x) {
  p1.flipX(x);
  p2.flipX(x);
}


void Pad::flipY(double y) {
  p1.flipY(y);
  p2.flipY(y);
}
