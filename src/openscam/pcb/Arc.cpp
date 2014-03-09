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

#include "Arc.h"

using namespace std;
using namespace OpenSCAM::PCB;


void Arc::rotate(const Point &center, double angle) {
  p.rotate(center, angle);
}


void Arc::translate(const Point &t) {
  p += t;
}


void Arc::multiply(double m) {
  p.multiply(m);
  width *= m;
  height *= m;
  thickness *= m;
  clearance *= m;
}


void Arc::round(int i) {
  p.round(i);
  Object::round(width, i);
  Object::round(height, i);
  Object::round(thickness, i);
  Object::round(clearance, i);
}


void Arc::write(ostream &stream) const {
  stream << "Arc[" << p << " " << width << " " << height << " "
         << thickness << " " << clearance << " "
         << startAngle << " " << deltaAngle << " \"" << flags << "\"]";
}


void Arc::read(istream &stream) {
  stream >> p >> width >> height >> thickness >> clearance >> startAngle
         >> deltaAngle;
  flags = readString(stream);
}


void Arc::bounds(Point &min, Point &max) const {
  // TODO compute this
}


void Arc::flipX(double x) {
  p.flipX(x);
  startAngle = (startAngle + deltaAngle + 180) % 360;
}


void Arc::flipY(double y) {
  p.flipY(y);
  startAngle = (startAngle + deltaAngle + 180) % 360;
}
