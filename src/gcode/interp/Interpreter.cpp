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

#include "Interpreter.h"

#include <gcode/parse/Parser.h>
#include <gcode/parse/Tokenizer.h>

#include <cbang/SStream.h>
#include <cbang/util/SmartToggle.h>
#include <cbang/log/SmartLogPrefix.h>

using namespace std;
using namespace cb;
using namespace GCode;


void Interpreter::addOverride(const Code &code, const string &gcode) {
  overrides[code] = gcode;
}


unsigned Interpreter::run(unsigned maxErrors) {
  unsigned errors = 0;

  while (hasMore())
    try {
      (*this)(next());

    } catch (const EndProgram &) {
      unwind();

    } catch (const Exception &e) {
      LOG_ERROR(e);
      if (maxErrors < ++errors) THROW("Too many errors aborting");
    }

  return errors;
}


void Interpreter::execute(const Code &code, int vars) {
  class SmartToggleProducer : public SmartToggle, public Producer {
  public:
    SmartToggleProducer(bool &toggle) : SmartToggle(toggle) {}

    // From Producer
    bool hasMore() const {return false;}
    cb::SmartPointer<Block> next() {THROW("Invalid");}
  };


  if (!inOverride) {
    auto it = overrides.find(code);

    if (it != overrides.end()) {
      push(new SmartToggleProducer(inOverride));
      push(it->second, SSTR("<" << code << ">"));
      return;
    }
  }

  GCodeInterpreter::execute(code, vars);
}


void Interpreter::read(const InputSource &source) {
  push(source);
  run();
}


void Interpreter::operator()(const SmartPointer<Block> &block) {
  if (block->isDeleted()) return;

  FileLocation location = block->getLocation().getStart();
  location.setCol(-1);
  SmartLogPrefix prefix(SSTR(location << ":"));

  try {
    OCodeInterpreter::operator()(block);

  } catch (const Exception &e) {
    throw Exception(SSTR(e.getMessage() << "\nWhile executing GCode block:"
                         << *block), location, e);
  }
}
