/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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
#include <cbang/util/SmartDepth.h>
#include <cbang/log/SmartLogThreadPrefix.h>

using namespace cb;
using namespace GCode;


unsigned Interpreter::run(unsigned maxErrors) {
  unsigned errors = 0;

  while (hasMore())
    try {
      next();

    } catch (const Exception &e) {
      LOG_ERROR(e);
      if (maxErrors < ++errors) THROW("Too many errors aborting");
    }

  return errors;
}


void Interpreter::read(const InputSource &source) {
  push(source);
  run();
}


void Interpreter::operator()(const SmartPointer<Block> &block) {
  if (block->isDeleted()) return;

  FileLocation location = block->getLocation().getStart();
  location.setCol(-1);
  SmartLogThreadPrefix prefix(SSTR(location << ":"));

  try {
    OCodeInterpreter::operator()(block);

  } catch (const Exception &e) {
    throw Exception(SSTR(e.getMessage() << "\nWhile executing GCode block:"
                         << *block), location, e);
  }
}
