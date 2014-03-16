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

#include "OCodeInterpreter.h"

#include "ast/Program.h"
#include "ast/OCode.h"

#include "Parser.h"

#include <cbang/log/Logger.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;

namespace {
  struct ReturnException {unsigned n; ReturnException(unsigned n) : n(n) {}};
  struct BreakException {unsigned n; BreakException(unsigned n) : n(n) {}};
  struct ContinueException
  {unsigned n; ContinueException(unsigned n) : n(n) {}};
}


OCodeInterpreter::OCodeInterpreter(Controller &controller,
                                   const cb::SmartPointer<Task> &task) :
  GCodeInterpreter(controller), task(task), condition(true) {}


OCodeInterpreter::~OCodeInterpreter() {} // Hide member destructors


void OCodeInterpreter::checkExpressions(OCode *ocode, const char *name,
                                        unsigned count) {
  const OCode::expressions_t &expressions = ocode->getExpressions();

  if (expressions.size() != count) {
    if (count)
      LOG_WARNING("'" << name << "' should have exactly " << count
                  << "expression" << (1 < count ? "s" : ""));
    else LOG_WARNING("'" << name << "' should not have any expressions");
  }
}


void OCodeInterpreter::upScope() {
  stack.pop_back();
  nameStack.pop_back();
}


void OCodeInterpreter::downScope() {
  stack.push_back(vector<double>(30));
  nameStack.push_back(name_map_t());
  if (stack.size() == 11) LOG_WARNING("exceeded recursion depth 10");
}


void OCodeInterpreter::doSub(OCode *ocode) {
  checkExpressions(ocode, "sub", 0);

  if (!subroutine.isNull()) THROWS("Nested subroutines not allowed");
  subroutine = new Program;

  if (ocode->getFilename().empty()) {
    unsigned number = ocode->getNumber();

    if (subroutines.find(number) != subroutines.end())
      LOG_WARNING("redefinition of subroutine " << number);

    subroutines[number] = subroutine;
    subroutineNumber = number;

  } else {
    string name = ocode->getFilename();

    if (namedSubroutines.find(name) != namedSubroutines.end())
      LOG_WARNING("redefinition of subroutine " << name);

    namedSubroutines[name] = subroutine;
    subroutineName = name;
  }
}


void OCodeInterpreter::doEndSub(OCode *ocode) {
  checkExpressions(ocode, "endsub", 0);

  if (subroutineName.empty()) {
    if (subroutineNumber != ocode->getNumber())
      LOG_WARNING("endsub number does not match");

  } else if (subroutineName != ocode->getFilename())
    LOG_WARNING("endsub name does not match");

  subroutine = 0;
  subroutineName = "";
}


void OCodeInterpreter::doCall(OCode *ocode) {
  // Eval args in parent scope
  const OCode::expressions_t &expressions = ocode->getExpressions();
  if (30 < expressions.size()) LOG_WARNING("more than 30 arguments");
  vector<double> args;
  for (unsigned i = 0; i < expressions.size() && i < 30; i++)
    args.push_back(expressions[i]->eval(*this));

  // Variable scoping
  downScope();
  SmartFunctor<OCodeInterpreter> closeScope(this, &OCodeInterpreter::upScope);

  // Set args as local variables
  unsigned i = 0;
  for (; i < expressions.size() && i < 30; i++) setReference(i + 1, args[i]);
  for (; i < 30; i++) setReference(i + 1, GCodeInterpreter::lookupReference(i));

  try {
    // Find subroute and call
    if (ocode->getFilename().empty()) {
      unsigned number = ocode->getNumber();
      subroutines_t::iterator it = subroutines.find(number);

      if (it == subroutines.end())
        THROWS("Subroutine " << number << " not found");
      it->second->process(*this);

    } else {
      string name = ocode->getFilename();
      named_subroutines_t::iterator it = namedSubroutines.find(name);

      if (it == namedSubroutines.end()) {
        // Try to load subroutine from file
        const char *scriptPath = SystemUtilities::getenv("GCODE_SCRIPT_PATH");
        if (!scriptPath)
          LOG_WARNING("Environment variable GCODE_SCRIPT_PATH not set");

        else {
          if (loadedFiles.insert(name).second) {
            string path = SystemUtilities::findInPath(scriptPath, name);
            InputSource source(path);
            Parser(task).parse(InputSource(path), *this);
          }

          // Look up again
          it = namedSubroutines.find(name);
        }
      }

      if (it == namedSubroutines.end())
        THROWS("Subroutine " << name << " not found");

      it->second->process(*this);
    }

  } catch (const ReturnException &e) {
    if (e.n != ocode->getNumber())
      LOG_WARNING("Return number does not match subroutine");
  }
}


void OCodeInterpreter::doReturn(OCode *ocode) {
  checkExpressions(ocode, "return", 0);
  throw ReturnException(ocode->getNumber());
}


void OCodeInterpreter::doDo(OCode *ocode) {
  checkExpressions(ocode, "do", 0);
  loopNumber = ocode->getNumber();
  loop = new Program;
  loopEnd = "while";
}


void OCodeInterpreter::doWhile(OCode *ocode) {
  checkExpressions(ocode, "while", 1);

  const OCode::expressions_t &expressions = ocode->getExpressions();
  cb::SmartPointer<Entity> expr = expressions.empty() ? 0 : expressions[0];

  if (loopEnd == "while" && loopNumber == ocode->getNumber()) {
    SmartPointer<Program> loop = this->loop;
    this->loop = 0;

    do {
      try {
        loop->process(*this);

      } catch (const ContinueException &e) {
        if (e.n == ocode->getNumber()) continue;
        throw;

      } catch (const BreakException &e) {
        if (e.n == ocode->getNumber()) break;
        throw;
      }
    } while(!expr.isNull() && expr->eval(*this));

  } else {
    loopNumber = ocode->getNumber();
    loop = new Program;
    loopEnd = "endwhile";
    loopExpr = expr;
  }
}


void OCodeInterpreter::doEndWhile(OCode *ocode) {
  checkExpressions(ocode, "endwhile", 0);

  SmartPointer<Program> loop = this->loop;
  SmartPointer<Entity> expr = loopExpr;

  this->loop = 0;
  loopExpr = 0;

  while (!expr.isNull() && expr->eval(*this))
    try {
      loop->process(*this);

    } catch (const ContinueException &e) {
      if (e.n == ocode->getNumber()) continue;
      throw;

    } catch (const BreakException &e) {
      if (e.n == ocode->getNumber()) break;
      throw;
    }
}


void OCodeInterpreter::doBreak(OCode *ocode) {
  checkExpressions(ocode, "break", 0);
  throw BreakException(ocode->getNumber());
}


void OCodeInterpreter::doContinue(OCode *ocode) {
  checkExpressions(ocode, "continue", 0);
  throw ContinueException(ocode->getNumber());
}


void OCodeInterpreter::doIf(OCode *ocode) {
  checkExpressions(ocode, "if", 1);

  const OCode::expressions_t &expressions = ocode->getExpressions();
  conditions.push_back(ocode->getNumber());
  if (!expressions.empty() && !expressions[0]->eval(*this)) condition = false;
}


void OCodeInterpreter::doElse(OCode *ocode) {
  checkExpressions(ocode, "else", 0);
  if (conditions.empty() || conditions.back() != ocode->getNumber())
    LOG_WARNING("Mismatched else");
  else condition = !condition;
}


void OCodeInterpreter::doEndIf(OCode *ocode) {
  checkExpressions(ocode, "endif", 0);
  if (conditions.empty() || conditions.back() != ocode->getNumber())
    LOG_WARNING("Mismatched endif");

  else {
    conditions.pop_back();
    condition = true;
  }
}


void OCodeInterpreter::doRepeat(OCode *ocode) {
  checkExpressions(ocode, "repeat", 1);
  loopNumber = ocode->getNumber();
  loop = new Program;
  loopEnd = "endrepeat";
  const OCode::expressions_t &expressions = ocode->getExpressions();
  repeat = expressions.empty() ? 0 : expressions[0]->eval(*this);
}


void OCodeInterpreter::doEndRepeat(OCode *ocode) {
  checkExpressions(ocode, "endrepeat", 0);

  SmartPointer<Program> loop = this->loop;
  this->loop = 0;
  unsigned repeat = this->repeat;
  for (unsigned i = 0; i < repeat; i++) loop->process(*this);
}


void OCodeInterpreter::operator()(const SmartPointer<Block> &block) {
  if (block->isDeleted()) return;

  OCode *ocode = block->findOCode();
  unsigned number = ocode ? ocode->eval(*this) : 0;
  string keyword = ocode ? ocode->getKeyword() : string();

  // Subroutine
  if (!subroutine.isNull() &&
      ((number != subroutineNumber && ocode &&
        ocode->getFilename() != subroutineName) || keyword != "endsub")) {
    subroutine->push_back(block);
    return;
  }

  // Loop
  if (!loop.isNull() && (number != loopNumber || keyword != loopEnd)) {
    loop->push_back(block);
    return;
  }

  // Condition
  if (!conditions.empty() && !condition &&
      (number != conditions.back() ||
       (keyword != "else" && keyword != "endif"))) return;

  if (!ocode) GCodeInterpreter::operator()(block);
  else {
    if (!ocode->getFilename().empty() && keyword != "sub" &&
        keyword != "call" && keyword != "endsub")
      LOG_WARNING("cannot specify file name on " << keyword);

    if (keyword == "sub") doSub(ocode);
    else if (keyword == "endsub") doEndSub(ocode);
    else if (keyword == "call") doCall(ocode);
    else if (keyword == "return") doReturn(ocode);
    else if (keyword == "do") doDo(ocode);
    else if (keyword == "while") doWhile(ocode);
    else if (keyword == "endwhile") doEndWhile(ocode);
    else if (keyword == "break") doBreak(ocode);
    else if (keyword == "continue") doContinue(ocode);
    else if (keyword == "if") doIf(ocode);
    else if (keyword == "else") doElse(ocode);
    else if (keyword == "endif") doEndIf(ocode);
    else if (keyword == "repeat") doRepeat(ocode);
    else if (keyword == "endrepeat") doEndRepeat(ocode);
    else LOG_WARNING("Unsupported O-Code: " << keyword);
  }
}


void OCodeInterpreter::setReference(unsigned num, double value) {
  if (!num || 30 < num || stack.empty())
    GCodeInterpreter::setReference(num, value);

  else {
    LOG_DEBUG(3, "Set local variable #" << num << " = " << value);
    stack.back()[num - 1] = value; // Local variable assignment
  }
}


void OCodeInterpreter::setReference(const string &name, double value) {
  if (nameStack.empty() || name[0] == '_')
    GCodeInterpreter::setReference(name, value);

  else {
    LOG_DEBUG(3, "Set local variable #<" << name << "> = " << value);
    nameStack.back()[name] = value; // Local variable assignment
  }
}


double OCodeInterpreter::lookupReference(unsigned num) {
  if (!num || 30 < num || stack.empty())
    return GCodeInterpreter::lookupReference(num);

  // Local variable reference
  return stack.back()[num - 1];
}


double OCodeInterpreter::lookupReference(const string &name) {
  if (name[0] != '_' && !nameStack.empty()) {
    name_map_t &nameMap = nameStack.back();
    name_map_t::iterator it = nameMap.find(name);
    if (it != nameMap.end()) return it->second;
    THROWS("Local reference to '" << name << "' not found");
  }

  return GCodeInterpreter::lookupReference(name);
}
