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

#ifndef OPENSCAM_PCB_OBJECT_H
#define OPENSCAM_PCB_OBJECT_H

#include <openscam/Real.h>

#include <iostream>
#include <string>

namespace OpenSCAM {
  namespace PCB {
    class Point;

    class Object {
    public:
      std::string prefix;

      Object() : prefix("\n") {}
      virtual ~Object() {} // Compiler needs this

      virtual const char *getName() const = 0;

      virtual void rotate(const Point &center, double angle) = 0;
      virtual void translate(const Point &t) = 0;
      virtual void multiply(double m) = 0;
      virtual void round(int x) = 0;
      virtual void write(std::ostream &stream) const = 0;
      virtual void read(std::istream &stream) = 0;
      virtual void bounds(Point &minPt, Point &maxPt) const = 0;
      virtual void flipX(double x) = 0;
      virtual void flipY(double x) = 0;
      virtual void lineSize(int size) {}
      virtual void textScale(int scale) {}

      void round(double &v, int i);
      void round(int &v, int i);

      void readWS(std::istream &stream) const;
      std::string readString(std::istream &stream);
      int readInt(std::istream &stream);
    };


    inline std::ostream &operator<<(std::ostream &stream, const Object &o) {
      o.write(stream);
      return stream;
    }


    inline std::istream &operator>>(std::istream &stream, Object &o) {
      o.read(stream);
      return stream;
    }
  }
}

#endif // OPENSCAM_PCB_OBJECT_H

