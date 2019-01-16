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

#include <cbang/String.h>


namespace GCode {
  class OCode : public Entity {
  public:
    typedef std::vector<cb::SmartPointer<Entity> > expressions_t;

  protected:
    cb::SmartPointer<Entity> numExpr;
    std::string filename;
    std::string keyword;

    unsigned number;
    expressions_t expressions;

  public:
    OCode(const cb::SmartPointer<Entity> numExpr, const std::string &keyword) :
      numExpr(numExpr), keyword(cb::String::toLower(keyword)), number(0) {}

    OCode(const std::string &filename, const std::string &keyword) :
      filename(filename), keyword(cb::String::toLower(keyword)),
      number(0) {}

    cb::SmartPointer<Entity> getNumExpression() const {return numExpr;}
    const std::string &getFilename() const {return filename;}
    const std::string &getKeyword() const {return keyword;}
    unsigned getNumber() const {return number;}
    const expressions_t &getExpressions() const {return expressions;}

    void addExpression(const cb::SmartPointer<Entity> &expr)
    {expressions.push_back(expr);}

    // From Entity
    double eval(Evaluator &evaluator);
    void print(std::ostream &stream) const;
  };
}
