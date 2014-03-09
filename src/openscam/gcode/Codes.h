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

#ifndef OPENSCAM_CODES_H
#define OPENSCAM_CODES_H

#include "ModalGroup.h"
#include "VarTypes.h"

#include <ostream>

namespace OpenSCAM {
  class Code {
  public:
    char type;
    float number;
    unsigned priority;
    ModalGroup group;
    int vars;
    const char *description;
  };

  std::ostream &operator<<(std::ostream &stream, const Code &code);

  class Codes {
  public:
    static const Code codes[];
    static const Code gcodes[];
    static const Code g10codes[];
    static const Code mcodes[];

    static const Code *find(char type, float number, float L = 0);
  };
}

#endif // OPENSCAM_CODES_H

