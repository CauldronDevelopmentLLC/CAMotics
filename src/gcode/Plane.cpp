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

#include "Plane.h"

#include <cbang/Exception.h>

using namespace GCode;


Plane::Plane(plane_t plane) : plane(plane) {
  // TODO Support UVW planes
  switch (plane) {
  case XY: case XZ: case YZ: break;
  case UV: case UW: case VW: default:
    THROW("Unsupported plane: " << plane);
  }
}


const char *Plane::getAxes() const {
  switch (plane) {
  case XY: return "XYZ";
  case XZ: return "XZY";
  case YZ: return "YZX";
  case UV: return "UVW";
  case UW: return "UWV";
  case VW: return "VWU";
  }
  THROW("Unsupported plane: " << plane);
}


const char *Plane::getOffsets() const {
  switch (plane) {
  case XY: return "IJ";
  case XZ: return "IK";
  case YZ: return "JK";
  case UV: return "IJ";
  case UW: return "IK";
  case VW: return "JK";
  }
  THROW("Unsupported plane: " << plane);
}


const unsigned *Plane::getAxisIndex() const {
  static const unsigned indices[][3] = {
    {0, 1, 2}, // XYZ
    {0, 2, 1}, // ZYX
    {1, 2, 0}, // YZX
    {6, 7, 8}, // UVW
    {6, 8, 7}, // UWV
    {7, 8, 6}, // VWU
  };

  switch (plane) {
  case XY: return indices[0];
  case XZ: return indices[1];
  case YZ: return indices[2];
  case UV: return indices[3];
  case UW: return indices[4];
  case VW: return indices[5];
  }
  THROW("Unsupported plane: " << plane);
}
