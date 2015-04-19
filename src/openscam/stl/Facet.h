/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#ifndef OPENSCAM_FACET_H
#define OPENSCAM_FACET_H

#include <openscam/Geom.h>

#include <ostream>

namespace OpenSCAM {
  class Facet : public cb::Triangle3F {
    cb::Vector3F normal;

  public:
    Facet(const cb::Vector3F &p1, const cb::Vector3F &p2,
          const cb::Vector3F &p3, const cb::Vector3F &normal) :
      cb::Triangle3F(p1, p2, p3), normal(normal) {}

    const cb::Vector3F &getNormal() const {return normal;}
    void setNormal(const cb::Vector3F &normal) {this->normal = normal;}
  };
}

#endif // OPENSCAM_FACET_H

