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

#ifndef OPENSCAM_INTERPRETER_H
#define OPENSCAM_INTERPRETER_H

#include "OCodeInterpreter.h"

#include <cbang/SmartPointer.h>
#include <cbang/io/Reader.h>


namespace OpenSCAM {
  class Interpreter : public OCodeInterpreter, public cb::Reader {
  public:
    Interpreter(Controller &controller,
                const cb::SmartPointer<Task> &task = new Task) :
      OCodeInterpreter(controller, task) {}

    // From cb::Reader
    void read(const cb::InputSource &source);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);
  };
}

#endif // OPENSCAM_INTERPRETER_H
