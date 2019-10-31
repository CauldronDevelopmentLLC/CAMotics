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


#include "Evaluator.h"

#include <gcode/VarTypes.h>
#include <gcode/ModalGroup.h>
#include <gcode/Processor.h>

#include <gcode/Controller.h>


namespace GCode {
  class Code;
  class Word;

  class GCodeInterpreter :
    public Processor, public Evaluator, public VarTypes, public ModalGroup {
  protected:
    Controller &controller;

    std::vector<cb::SmartPointer<std::ostream> > log;

  public:
    GCodeInterpreter(Controller &controller);

    virtual void setReference(address_t addr, double value);
    virtual void setReference(const std::string &name, double value);
    virtual void clearReference(const std::string &name);

    virtual void execute(const Code &code, int vars);

    void specialComment(const std::string text);
    std::string interpolate(const std::string &s);
    std::string canonical(const std::string &name) const;

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

    // From Evaluator
    double lookupReference(address_t addr);
    double lookupReference(const std::string &name);
    bool hasReference(const std::string &name);
  };
}
