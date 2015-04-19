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

#include "Word.h"

#include "Number.h"

#include <openscam/gcode/Codes.h>

#include <cctype>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Word::Word(const Code *code) :
  type(code->type), expr(new Number(code->number)), value(code->number),
  code(code) {}


double Word::eval(Evaluator &evaluator) {
  value = expr->eval(evaluator);
  code = Codes::find(type, value);
  return value;
}


void Word::print(ostream &stream) const {
  stream << (char)toupper(type) << *expr;
}
