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

#include "Evaluator.h"

#include "ast/UnaryOp.h"
#include "ast/BinaryOp.h"
#include "ast/QuotedExpr.h"
#include "ast/Reference.h"
#include "ast/NamedReference.h"
#include "ast/FunctionCall.h"
#include "ast/Number.h"

#include <camotics/sim/Addresses.h>

#include <cbang/Exception.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


double Evaluator::lookupReference(unsigned num) {
  THROW("Named reference not supported");
}


double Evaluator::lookupReference(const string &name) {
  THROW("Numerical reference not supported");
}


double Evaluator::eval(UnaryOp &e) {
  double value = e.getExpr()->eval(*this);

  switch (e.getType()) {
  case Operator::ADD_OP: return value;
  case Operator::SUB_OP: return -value;
  default: THROWS(e.getLocation() << " Invalid unary operator");
  }
}


double Evaluator::eval(BinaryOp &e) {
  double left = e.getLeft()->eval(*this);
  double right = e.getRight()->eval(*this);

  if (right == 0 &&
      (e.getType() == Operator::DIV_OP || e.getType() == Operator::MOD_OP)) {
    LOG_ERROR(e.getLocation() << ": Divide by zero");
    return 0;
  }

  switch (e.getType()) {
  case Operator::EXP_OP: return pow(left, right);
  case Operator::MUL_OP: return left * right;
  case Operator::DIV_OP: return left / right;
  case Operator::MOD_OP: return fmod(left, right);
  case Operator::ADD_OP: return left + right;
  case Operator::SUB_OP: return left - right;
  case Operator::EQ_OP: return left == right;
  case Operator::NE_OP: return left != right;
  case Operator::GT_OP: return left > right;
  case Operator::GE_OP: return left >= right;
  case Operator::LT_OP: return left < right;
  case Operator::LE_OP: return left <= right;
  case Operator::AND_OP: return left && right;
  case Operator::OR_OP: return left || right;
  case Operator::XOR_OP: return (bool)left ^ (bool)right;
  default: THROWS(e.getLocation() << " Invalid binary operator");
  }
}


double Evaluator::eval(QuotedExpr &e) {
  return e.getExpression()->eval(*this);
}


double Evaluator::eval(FunctionCall &e) {
  string name = String::toUpper(e.getName());
  double arg1 = e.getArg1()->eval(*this);

  if (e.getArg2().isNull()) {
    if (name == "ABS") return fabs(arg1);
    if (name == "ACOS") return acos(arg1) * 180.0 / M_PI;
    if (name == "ASIN") return asin(arg1) * 180.0 / M_PI;
    if (name == "COS") return cos(arg1 * M_PI / 180.0);
    if (name == "EXP") return exp(arg1);
    if (name == "FIX") return floor(arg1);
    if (name == "FUP") return ceil(arg1);
    if (name == "ROUND") return Math::round(arg1);
    if (name == "LN") return log(arg1);
    if (name == "SIN") return sin(arg1 * M_PI / 180.0);
    if (name == "SQRT") return sqrt(arg1);
    if (name == "TAN") return tan(arg1 * M_PI / 180.0);

  } else {
    double arg2 = e.getArg2()->eval(*this);
    if (name == "ATAN") return atan2(arg1, arg2) * 180.0 / M_PI;
  }

  THROWS(e.getLocation() << " Unsupported function '" << name << "'");
}


double Evaluator::eval(NamedReference &e) {
  return lookupReference(e.getName());
}


double Evaluator::eval(Reference &e) {
  double num = e.getExpression()->eval(*this);
  
  if (num < 1 || MAX_ADDRESS < num || ((unsigned)num) != num)
    THROWS(e.getLocation() << " Invalid reference number " << num);

  return lookupReference((unsigned)num);
}


double Evaluator::eval(Number &e) {
  return e.getValue();
}


SmartPointer<Entity> Evaluator::reduce(const SmartPointer<Entity> &entity) {
  if (entity->isConstant()) return new Number(entity->eval(*this));
  return entity;
}
