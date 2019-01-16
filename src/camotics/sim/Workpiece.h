/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#pragma once


#include <camotics/contour/FieldFunction.h>


namespace CAMotics {
  class Workpiece : public cb::Rectangle3D, public FieldFunction {
    cb::Vector3D center;
    cb::Vector3D halfDim2;

  public:
    Workpiece(const cb::Rectangle3D &r = cb::Rectangle3D());

    cb::Rectangle3D getBounds() const {return *this;}
    bool isValid() const {return getVolume();}
    using cb::Rectangle3D::contains;

    // From FieldFunction
    double depth(const cb::Vector3D &p) const;
  };
}
