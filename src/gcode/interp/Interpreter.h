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

#include "OCodeInterpreter.h"

#include <cbang/SmartPointer.h>
#include <cbang/io/Reader.h>


namespace GCode {
  class Tokenizer;

  class Interpreter : public OCodeInterpreter, public cb::Reader {
    bool inOverride;
    std::map<const Code, std::string> overrides;

  public:
    Interpreter(Controller &controller) :
      OCodeInterpreter(controller), inOverride(false) {}

    void addOverride(const Code &code, const std::string &gcode);

    unsigned run(unsigned maxErrors = 32);

    // From GCodeInterpreter
    void execute(const Code &code, int vars);

    // From cb::Reader
    void read(const cb::InputSource &source);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);
  };
}
