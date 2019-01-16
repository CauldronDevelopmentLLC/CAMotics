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

#include <gcode/Producer.h>
#include <gcode/ast/Program.h>


namespace GCode {
  class Loop : public Producer {
    unsigned number;
    cb::SmartPointer<Program> program;
    unsigned i;

  public:
    Loop(unsigned number, const cb::SmartPointer<Program> &program);

    unsigned getNumber() const {return number;}

    bool isEmpty() const {return program->empty();}
    bool atStart() const {return i == 0;}

    void continueLoop() {i = 0;}

    // From Producer
    cb::SmartPointer<Block> next();
  };
}
