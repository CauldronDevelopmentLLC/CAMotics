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


#include "Sweep.h"

namespace CAMotics {
  class ConicSweep : public Sweep {
    const double l;  // Length
    const double rt; // Radius 1
    const double rb; // Radius 1
    const double Tm; // GCode::Tool slope

  public:
    ConicSweep(double length, double radius1, double radius2 = -1);

    // From Sweep
    void getBBoxes(const cb::Vector3D &start, const cb::Vector3D &end,
                   std::vector<cb::Rectangle3D> &bboxes,
                   double tolerance) const;
    double depth(const cb::Vector3D &start, const cb::Vector3D &end,
               const cb::Vector3D &p) const;
  };
}
