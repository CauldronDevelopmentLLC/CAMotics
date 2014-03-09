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

#include "Polygon.h"

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM::PCB;


Polygon::Polygon() {}


void Polygon::rotate(const Point &center, double angle) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].rotate(center, angle);
}


void Polygon::translate(const Point &t) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i] += t;
}


void Polygon::multiply(double m) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].multiply(m);
}


void Polygon::round(int i) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].round(i);
}


void Polygon::write(ostream &stream) const {
  stream << "Polygon(\"" << flags << "\") (\n    ";
  for (unsigned int i = 0; i < points.size(); i++)
    stream << "[" << points[i] << "]  ";
  stream << endl << ")" << endl;
}


void Polygon::read(istream &stream) {
  flags = readString(stream);

  readWS(stream);
  int c = stream.get();
  if (c != ')')  THROW("missing ')' on Polygon");

  readWS(stream);
  c = stream.get();
  if (c != '(')  THROW("missing '(' on Polygon");

  do {
    readWS(stream);
    c = stream.get();
    if (c != '[') break;

    Point p;
    stream >> p;
    points.push_back(p);

    readWS(stream);
    c = stream.get();
    if (c != ']')  THROW("missing ']' on Polygon point");

  } while (true);

  stream.unget();
}


void Polygon::bounds(Point &min, Point &max) const {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].bounds(min, max);
}


void Polygon::flipX(double x) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].flipX(x);
}


void Polygon::flipY(double y) {
  for (unsigned int i = 0; i < points.size(); i++)
    points[i].flipY(y);
}
