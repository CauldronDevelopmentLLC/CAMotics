/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_GCODE_INTERPRETER_H
#define OPENSCAM_GCODE_INTERPRETER_H

#include "Processor.h"

#include "VarTypes.h"
#include "ModalGroup.h"
#include "Evaluator.h"

#include <openscam/sim/Controller.h>

namespace OpenSCAM {
  class Code;
  class Word;

  class GCodeInterpreter :
    public Processor, public Evaluator, public VarTypes, public ModalGroup {
    Controller &controller;

  public:
    GCodeInterpreter(Controller &controller);

    Controller &getController() {return controller;}

    virtual void setReference(unsigned num, double value);
    virtual void setReference(const std::string &name, double value);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

    // From Evaluator
    double lookupReference(unsigned num);
    double lookupReference(const std::string &name);
  };
}

#endif // OPENSCAM_GCODE_INTERPRETER_H
