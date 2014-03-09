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

#ifndef OPENSCAM_PCB_ELEMENT_H
#define OPENSCAM_PCB_ELEMENT_H

#include "Layout.h"
#include "Point.h"

#include <string>

namespace OpenSCAM {
  namespace PCB {
    class Element : public Layout {
      std::string flags;
      std::string desc;
      std::string name;
      std::string value;
      Point m;
      Point t;
      int tdir;
      int tscale;
      std::string tflags;

    public:
      // From Object
      const char *getName() const {return "Element";}
      void rotate(const Point &center, double angle);
      void translate(const Point &t);
      void multiply(double m);
      void round(int x);
      void write(std::ostream &stream) const;
      void read(std::istream &stream);
      void bounds(Point &min, Point &max) const;
      void flipX(double x);
      void flipY(double x);
      void textScale(int scale);

      void centerMark();
      void subtractMask(int clear);
    };
  }
}

#endif // OPENSCAM_PCB_ELEMENT_H

