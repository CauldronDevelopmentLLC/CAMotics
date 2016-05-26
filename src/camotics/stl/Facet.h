/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_FACET_H
#define CAMOTICS_FACET_H

#include <camotics/Geom.h>

#include <ostream>

namespace CAMotics {
  class Facet : public cb::Triangle3F {
    cb::Vector3F normal;

  public:
    Facet() : cb::Triangle3F(cb::Vector3F()) {}
    Facet(const cb::Vector3F &p1, const cb::Vector3F &p2,
          const cb::Vector3F &p3, const cb::Vector3F &normal) :
      cb::Triangle3F(p1, p2, p3), normal(normal) {}

    const cb::Vector3F &getNormal() const {return normal;}
    cb::Vector3F &getNormal() {return normal;}
    void setNormal(const cb::Vector3F &normal) {this->normal = normal;}
  };
}

#endif // CAMOTICS_FACET_H
