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

#ifndef OPENSCAM_PCB_POINT_H
#define OPENSCAM_PCB_POINT_H

#include "Object.h"

#include <openscam/Geom.h>

#include <cbang/Math.h>


namespace OpenSCAM {
  namespace PCB {
    struct Point : public Object, public cb::Vector2D {
      Point(double x = 0, double y = 0) : cb::Vector2D(x, y) {}
      Point(const cb::Vector2D &p) : cb::Vector2D(p) {}

      // From Object
      const char *getName() const {return "Point";}

      void rotate(const Point &center, double angle) {
        // Translate
        cb::Vector2D t = *this - center;

        // Rotate
        x() = t.x() * cos(angle) + t.y() * sin(angle);
        y() = -t.x() * sin(angle) + t.y() * cos(angle);

        // Translate back
        *this += center;
      }

      void translate(const Point &t) {*this += t;}
      void multiply(double m) {x() *= m; y() *= m;}
      void round(int i) {Object::round(x(), i); Object::round(y(), i);}

      void write(std::ostream &stream) const {
        stream << (int)x() << " " << (int)y();
      }

      void read(std::istream &stream) {stream >> x() >> y();}

      void bounds(Point &min, Point &max) const {
        if (x() < min.x()) min.x() = x();
        if (y() < min.y()) min.y() = y();
        if (max.x() < x()) max.x() = x();
        if (max.y() < y()) max.y() = y();
      }

      void flipX(double x) {this->x() -= 2 * (this->x() - x);}
      void flipY(double y) {this->y() -= 2 * (this->y() - y);}
    };

    inline std::ostream &operator<<(std::ostream &stream, const Point &p) {
      p.write(stream);
      return stream;
    }
  }
}

#endif // OPENSCAM_PCB_POINT_H

