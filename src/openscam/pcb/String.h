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

#ifndef OPENSCAM_PCB_STRING_H
#define OPENSCAM_PCB_STRING_H

#include <string>

#include "Object.h"

namespace OpenSCAM {
  namespace PCB {
    class String : public Object {
      std::string s;

    public:
      String(const std::string &s) : s(s) {}

      // From Object
      const char *getName() const {return "String";}
      void rotate(const Point &center, double angle) {}
      void translate(const Point &t) {}
      void multiply(double m) {}
      void round(int i) {}
      void write(std::ostream &stream) const {stream << s;}
      void read(std::istream &stream) {}
      void bounds(Point &min, Point &max) const {}
      void flipX(double x) {}
      void flipY(double x) {}
    };
  }
}

#endif // OPENSCAM_PCB_STRING_H

