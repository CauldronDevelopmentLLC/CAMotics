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

#include <stdint.h>


namespace CAMotics {
  class CorrectedMC33Cube {
    uint8_t lutEntry;
    const double *cube;

  public:
    CorrectedMC33Cube(uint8_t lutEntry, const double cube[8]) :
      lutEntry(lutEntry), cube(cube) {}

    bool testFace(int8_t face) const;
    bool testInterior(uint8_t ccase, uint8_t config, int8_t s) const;
    int interiorAmbiguityEdge(int face, int s) const;
    int interiorAmbiguity(int face, int s) const;
    int testCase13() const;

    const int8_t *process(unsigned &count) const;
  };
}
