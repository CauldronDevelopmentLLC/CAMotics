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

#include "BinaryOp.h"

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace GCode;

BinaryOp::BinaryOp(Operator type, const SmartPointer<Entity> &left,
                   const SmartPointer<Entity> &right) :
  type(type), left(left), right(right) {

  location = LocationRange(left->getLocation().getStart(),
                           right->getLocation().getEnd());
}


void BinaryOp::print(ostream &stream) const {
  stream << *left << ' ';

  switch (type) {
  case Operator::EXP_OP: stream << "**";  break;
  case Operator::MUL_OP: stream << '*';   break;
  case Operator::DIV_OP: stream << '/';   break;
  case Operator::MOD_OP: stream << "MOD"; break;
  case Operator::ADD_OP: stream << '+';   break;
  case Operator::SUB_OP: stream << '-';   break;
  case Operator::EQ_OP:  stream << "EQ";  break;
  case Operator::NE_OP:  stream << "NE";  break;
  case Operator::GT_OP:  stream << "GT";  break;
  case Operator::GE_OP:  stream << "GE";  break;
  case Operator::LT_OP:  stream << "LT";  break;
  case Operator::LE_OP:  stream << "LE";  break;
  case Operator::AND_OP: stream << "AND"; break;
  case Operator::OR_OP:  stream << "OR";  break;
  case Operator::XOR_OP: stream << "XOR"; break;
  default: THROW("Invalid binary operator");
  }

  stream << ' ' << *right;
}
