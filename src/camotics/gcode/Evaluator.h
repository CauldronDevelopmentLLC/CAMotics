/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#ifndef CAMOTICS_EVALUATOR_H
#define CAMOTICS_EVALUATOR_H

#include <string>

#include <cbang/SmartPointer.h>


namespace CAMotics {
  class Entity;
  class UnaryOp;
  class BinaryOp;
  class FunctionCall;
  class NamedReference;
  class Number;
  class QuotedExpr;
  class Reference;

  class Evaluator {
  public:
    virtual double lookupReference(unsigned num);
    virtual double lookupReference(const std::string &name);
    virtual double eval(UnaryOp &e);
    virtual double eval(BinaryOp &e);
    virtual double eval(QuotedExpr &e);
    virtual double eval(FunctionCall &e);
    virtual double eval(NamedReference &e);
    virtual double eval(Reference &e);
    virtual double eval(Number &e);

    cb::SmartPointer<Entity> reduce(const cb::SmartPointer<Entity> &entity);
  };
}

#endif // CAMOTICS_EVALUATOR_H

