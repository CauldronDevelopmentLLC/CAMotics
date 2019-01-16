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


#include "Entity.h"

#include <cbang/SmartPointer.h>

#include <cctype>

namespace GCode {
  class Code;

  class Word : public Entity {
    char type;
    cb::SmartPointer<Entity> expr;
    double value;
    const Code *code;

  public:
    Word(const Code *code);
    Word(char type, const cb::SmartPointer<Entity> &expr) :
      type(std::toupper(type)), expr(expr), value(0), code(0) {}

    void setType(char type) {this->type = std::toupper(type);}
    char getType() const {return type;}

    void setExpression(const cb::SmartPointer<Entity> &expr)
    {this->expr = expr;}
    const cb::SmartPointer<Entity> &getExpression() const {return expr;}

    double getValue() const {return value;}
    const Code *getCode() const {return code;}

    // From Entity
    double eval(Evaluator &evaluator);
    void print(std::ostream &stream) const;
  };
}
