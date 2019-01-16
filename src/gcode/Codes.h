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


#include "ModalGroup.h"
#include "VarTypes.h"

#include <ostream>

namespace GCode {
  typedef enum {
    PRI_OWORD,
    PRI_COMMENT,
    PRI_MODE,
    PRI_FEED_RATE,
    PRI_SPEED,
    PRI_TOOL,
    PRI_IO,
    PRI_CHANGE_TOOL,
    PRI_SPINDLE,
    PRI_STATE,
    PRI_COOLANT,
    PRI_OVERRIDES,
    PRI_USER,
    PRI_DWELL,
    PRI_PLANE,
    PRI_UNITS,
    PRI_RADIUS_COMP,
    PRI_LENGTH_COMP,
    PRI_COORD_SYS,
    PRI_PATH,
    PRI_DIST,
    PRI_RETRACT,
    PRI_OFFSETS,
    PRI_MOTION,
    PRI_STOP,
  } code_priority_t;


  class Code {
  public:
    char type;
    unsigned number; // Code * 10
    code_priority_t priority;
    ModalGroup group;
    int vars;
    const char *description;

    bool operator<(const Code &o) const;

    static Code parse(const std::string &s);
  };

  std::ostream &operator<<(std::ostream &stream, const Code &code);


  class Codes {
  public:
    static const Code codes[];
    static const Code gcodes[];
    static const Code g10codes[];
    static const Code mcodes[];

    static const Code *find(char type, double number, double L = 0);
  };
}
