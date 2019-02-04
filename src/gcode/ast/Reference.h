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

#include <gcode/Addresses.h>

#include <cbang/SmartPointer.h>


namespace GCode {
  class Reference : public Entity {
    cb::SmartPointer<Entity> expr;
    address_t addr;

  public:
    Reference(const cb::SmartPointer<Entity> &expr) :
      expr(expr), addr((address_t)0) {}

    cb::SmartPointer<Entity> getExpression() const {return expr;}
    address_t getAddress() const {return addr;}

    address_t evalAddress(Evaluator &evaluator);  // Used by Assign

    // From Entity
    double eval(Evaluator &evaluator);
    void print(std::ostream &stream) const;
  };
}
