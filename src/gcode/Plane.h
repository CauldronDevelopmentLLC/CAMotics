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

#include <gcode/machine/MachineEnum.h>

namespace GCode {
  class Plane : public MachineEnum {
    plane_t plane;

  public:
    Plane(plane_t plane);

    const char *getAxes() const;
    const char *getOffsets() const;
    const unsigned *getAxisIndex() const;

    char getXAxis() const {return getAxes()[0];}
    char getYAxis() const {return getAxes()[1];}
    char getZAxis() const {return getAxes()[2];}
    unsigned getXVarType() const {return getVarType(getXAxis());}
    unsigned getYVarType() const {return getVarType(getYAxis());}
    unsigned getZVarType() const {return getVarType(getZAxis());}
  };
}
