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

#ifndef OPENSCAM_REFERENCE_H
#define OPENSCAM_REFERENCE_H

#include "Entity.h"

#include <cbang/SmartPointer.h>

namespace OpenSCAM {
  class Reference : public Entity {
    cb::SmartPointer<Entity> expr;
    double number;

  public:
    Reference(const cb::SmartPointer<Entity> &expr) : expr(expr), number(0) {}

    cb::SmartPointer<Entity> getExpression() const {return expr;}
    double getNumber() const {return number;}

    double evalNumber(Evaluator &evaluator);  // Used by Assign

    // From Entity
    double eval(Evaluator &evaluator);
    void print(std::ostream &stream) const;
  };
}

#endif // OPENSCAM_REFERENCE_H

