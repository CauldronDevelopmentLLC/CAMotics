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

#ifndef OPENSCAM_DXFARC_H
#define OPENSCAM_DXFARC_H

#include "DXFEntity.h"


namespace OpenSCAM {
  class DXFArc : public DXFEntity {
    cb::Vector3D center;
    double radius;
    double startAngle;
    double endAngle;
    bool clockwise;

  public:
    DXFArc(const cb::Vector3D &center, double radius, double startAngle,
           double endAngle, bool clockwise) :
      center(center), radius(radius), startAngle(startAngle),
      endAngle(endAngle), clockwise(clockwise) {}

    const cb::Vector3D &getCenter() const {return center;}
    double getRadius() const {return radius;}
    double getStartAngle() const {return startAngle;}
    double getEndAngle() const {return endAngle;}
    bool getClockwise() const {return clockwise;}

    // From DXFEntity
    type_t getType() const {return DXF_ARC;}
  };
}

#endif // OPENSCAM_DXFARC_H

