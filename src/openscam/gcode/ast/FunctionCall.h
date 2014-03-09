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

#ifndef OPENSCAM_FUNCTION_CALL_H
#define OPENSCAM_FUNCTION_CALL_H

#include "Entity.h"

#include <cbang/SmartPointer.h>

#include <string>

namespace OpenSCAM {
  class FunctionCall : public Entity {
    std::string name;
    cb::SmartPointer<Entity> arg1;
    cb::SmartPointer<Entity> arg2;

  public:
    FunctionCall(const std::string &name, const cb::SmartPointer<Entity> &arg1,
                 const cb::SmartPointer<Entity> &arg2 = 0) :
      name(name), arg1(arg1), arg2(arg2) {}

    const std::string &getName() const {return name;}
    const cb::SmartPointer<Entity> &getArg1() const {return arg1;}
    const cb::SmartPointer<Entity> &getArg2() const {return arg2;}

    // From Entity

    double eval(Evaluator &evaluator) {return evaluator.eval(*this);}
    void print(std::ostream &stream) const;
  };
}

#endif // OPENSCAM_FUNCTION_CALL_H

