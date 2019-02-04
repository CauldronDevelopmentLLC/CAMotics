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

#include "Assign.h"

#include "Reference.h"

using namespace std;
using namespace GCode;


double Assign::eval(Evaluator &evaluator) {
  exprValue = expr->eval(evaluator); // Evaluate the expression first

  // Only eval numeric reference's number expression
  if (ref->instance<Reference>())
    ref->instance<Reference>()->evalAddress(evaluator);

  return exprValue;
}


void Assign::print(ostream &stream) const {
  stream << *ref << " = " << *expr;
}
