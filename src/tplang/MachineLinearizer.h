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

#ifndef MACHINE_LINEARIZER_H
#define MACHINE_LINEARIZER_H

#include "MachineAdapter.h"


namespace tplang {
  class MachineLinearizer : public MachineAdapter {
    double arcPrecision;

  public:
    MachineLinearizer(double arcPrecision = 360) : arcPrecision(arcPrecision) {}

    // From tplang::MachineInterface
    void arc(const cb::Vector3D &offset, double degrees, plane_t plane);
  };
}

#endif // MACHINE_LINEARIZER_H

