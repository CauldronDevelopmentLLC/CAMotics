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

#include "Object.h"

#include <cbang/Exception.h>
#include <cbang/Math.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM::PCB;


void Object::round(double &v, int i) {
  v = i * Math::round(v / i);
}


void Object::round(int &v, int i) {
  v = i * (int)Math::round((double)v / i);
}


void Object::readWS(istream &stream) const {
  int c;
  do {
    c = stream.get();
    if (c == -1) return;
  } while (isspace(c));

  stream.unget();
}


string Object::readString(istream &stream) {
  readWS(stream);

  int c = stream.get();
  if (c != '"') THROW("Expected '\"'");
    
  string s;
  do {
    c = stream.get();
    if (c == '"') break;
    s += c;
  } while (!stream.fail());

  return s;
}


int Object::readInt(istream &stream) {
  readWS(stream);

  int c = stream.get();
  int sign = 1;

  if (c == '-') sign = -1;
  else if (!isdigit(c)) THROW("Expected number");
    
  int x = 0;
  do {
    x = 10 * x + c - '0';

    c = stream.get();
    if (!isdigit(c)) {
      stream.unget();
      break;
    }
  } while (!stream.fail());

  return x * sign;
}
