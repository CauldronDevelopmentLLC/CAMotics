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

#include "Interpreter.h"

#include "Parser.h"

#include <cbang/SStream.h>
#include <cbang/util/SmartDepth.h>
#include <cbang/log/SmartLogThreadPrefix.h>

using namespace cb;
using namespace OpenSCAM;


void Interpreter::read(const InputSource &source) {
  try {
    Parser(getTask()).parse(source, *this);
  } catch (const EndProgram &) {}
}


void Interpreter::operator()(const SmartPointer<Block> &block) {
  if (block->isDeleted()) return;

  FileLocation location = block->getLocation().getStart();
  location.setCol(-1);
  SmartLogThreadPrefix prefix(SSTR(location << ": "));

  OCodeInterpreter::operator()(block);
}
