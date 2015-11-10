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

#ifndef CAMOTICS_WORKPIECE_H
#define CAMOTICS_WORKPIECE_H

#include <camotics/Geom.h>

#include <camotics/contour/FieldFunction.h>


namespace CAMotics {
  class Workpiece : public Rectangle3R, public FieldFunction {
    Vector3R center;
    Vector3R halfDim2;

  public:
    Workpiece(const Rectangle3R &r);

    Rectangle3R getBounds() const {return *this;}
    bool isValid() const {return getVolume();}
    using Rectangle3R::contains;

    // From FieldFunction
    real depth(const Vector3R &p) const;
  };
}

#endif // CAMOTICS_WORKPIECE_H

