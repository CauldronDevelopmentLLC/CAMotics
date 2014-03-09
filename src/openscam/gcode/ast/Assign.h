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

#ifndef OPENSCAM_REFERENCE_ASSIGN_H
#define OPENSCAM_REFERENCE_ASSIGN_H

#include "Entity.h"
#include "Reference.h"

#include <cbang/SmartPointer.h>


namespace OpenSCAM {
  class Assign : public Entity {
    cb::SmartPointer<Entity> ref;
    cb::SmartPointer<Entity> expr;

    double exprValue;

  public:
    Assign(const cb::SmartPointer<Entity> &ref,
           const cb::SmartPointer<Entity> &expr) :
      ref(ref), expr(expr), exprValue(0) {}

    cb::SmartPointer<Entity> getReference() const {return ref;}
    cb::SmartPointer<Entity> getExpression() const {return expr;}

    double getExprValue() const {return exprValue;}

    // From Entity
    double eval(Evaluator &evaluator);
    void print(std::ostream &stream) const;
  };
}

#endif // OPENSCAM_REFERENCE_ASSIGN_H

