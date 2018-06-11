/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "SubroutineCall.h"
#include "SubroutineLoader.h"
#include "DoLoop.h"
#include "RepeatLoop.h"

#include <gcode/ast/Program.h>
#include <gcode/ast/OCode.h>

#include <gcode/parse/Parser.h>

#include <cbang/log/Logger.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace cb;
using namespace GCode;


OCodeInterpreter::OCodeInterpreter(Controller &controller) :
  GCodeInterpreter(controller), condition(true) {}


OCodeInterpreter::~OCodeInterpreter() {
  // Unwind stack in correct order
  while (!producers.empty()) producers.pop_back();
}


const SmartPointer<Program> &
OCodeInterpreter::lookupSubroutine(const string &name) const {
  auto it = namedSubroutines.find(name);
  if (it == namedSubroutines.end())
    THROWS("Subroutine " << name << " not found");
  return it->second;
}


void OCodeInterpreter::push(const SmartPointer<Producer> &producer) {
  producers.push_back(producer);
}


void OCodeInterpreter::push(const InputSource &source) {
  push(new Parser(source));
}

void OCodeInterpreter::push(Program &program) {
  push(new ProgramProducer(SmartPointer<Program>::Phony(&program)));
}


bool OCodeInterpreter::hasMore() {
  while (!producers.empty()) {
    if (producers.back()->hasMore()) return true;
    producers.pop_back();
  }

  return false;
}


void OCodeInterpreter::next() {
  if (producers.empty()) return;

  try {
    (*this)(producers.back()->next());

  } catch (const EndProgram &) {
    while (!producers.empty()) producers.pop_back();
  }
}


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
  controller.popScope();
}


void OCodeInterpreter::downScope() {
  stack.push_back(StackEntry());
  if (stack.size() == 101) LOG_WARNING("exceeded recursion depth 100");
  controller.pushScope();
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

  // Note, creating SubroutineCall affects variable scope
  SmartPointer<SubroutineCall> subroutineCall =
    new SubroutineCall(*this, ocode->getNumber());

  // Set args as local variables
  unsigned i = 0;
  for (; i < expressions.size() && i < 30; i++)
    setReference((address_t)(i + 1), args[i]);
  for (; i < 30; i++)
    setReference((address_t)(i + 1),
                 GCodeInterpreter::lookupReference((address_t)i));

  // Find subroute and call
  if (ocode->getFilename().empty()) {
    unsigned number = ocode->getNumber();
    subroutines_t::iterator it = subroutines.find(number);

    if (it == subroutines.end())
      THROWS("Subroutine " << number << " not found");
    subroutineCall->setProgram(it->second);

  } else {
    string name = ocode->getFilename();
    LOG_DEBUG(3, "Seeking subroutine \"" << name << '"');
    named_subroutines_t::iterator it = namedSubroutines.find(name);

    if (it != namedSubroutines.end())
      subroutineCall->setProgram(it->second);

    else {
      // Try to load subroutine from file
      const char *scriptPath = SystemUtilities::getenv("GCODE_SCRIPT_PATH");
      if (!scriptPath) {
        LOG_WARNING("Environment variable GCODE_SCRIPT_PATH not set");
        THROWS("Subroutine " << name << " not found");

      } else if (loadedFiles.insert(name).second) {
        string path = SystemUtilities::findInPath(scriptPath, name);
        if (path.empty())
          THROWS("Subroutine \"" << name
                 << "\" file not found in GCODE_SCRIPT_PATH");

        push(new SubroutineLoader(name, subroutineCall, *this));
        push(new Parser(InputSource(path)));
        return;
      }
    }
  }

  if (!subroutineCall->getProgram().isNull()) push(subroutineCall);
}


void OCodeInterpreter::doReturn(OCode *ocode) {
  checkExpressions(ocode, "return", 0);

  while (!producers.empty()) {
    SmartPointer<Producer> producer = producers.back();
    producers.pop_back();

    if (producer.isInstance<SubroutineCall>()) {
      if (producer.cast<SubroutineCall>()->getNumber() != ocode->getNumber())
        LOG_WARNING("Return number does not match subroutine");

      break;
    }
  }
}


void OCodeInterpreter::doDo(OCode *ocode) {
  checkExpressions(ocode, "do", 0);
  loop.number = ocode->getNumber();
  loop.program = new Program;
  loop.end = "while";
}


void OCodeInterpreter::doWhile(OCode *ocode) {
  checkExpressions(ocode, "while", 1);

  const OCode::expressions_t &expressions = ocode->getExpressions();
  SmartPointer<Entity> expr = expressions.empty() ? 0 : expressions[0];

  if (loop.end == "while" && loop.number == ocode->getNumber()) {
    push(new DoLoop(ocode->getNumber(), loop.program, *this, expr, true));
    loop.program = 0;

  } else {
    loop.number = ocode->getNumber();
    loop.program = new Program;
    loop.end = "endwhile";
    loop.expr = expr;
  }
}


void OCodeInterpreter::doEndWhile(OCode *ocode) {
  checkExpressions(ocode, "endwhile", 0);

  push(new DoLoop(ocode->getNumber(), loop.program, *this, loop.expr, false));
  loop.program = 0;
  loop.expr = 0;
}


void OCodeInterpreter::doBreak(OCode *ocode) {
  checkExpressions(ocode, "break", 0);

  while (!producers.empty()) {
    SmartPointer<Producer> producer = producers.back();
    if (!producer.isInstance<Loop>())
      THROWS("Break outside loop or OCode number mismatch");

    producers.pop_back();

    if (producer.cast<Loop>()->getNumber() != ocode->getNumber()) break;
  }
}


void OCodeInterpreter::doContinue(OCode *ocode) {
  checkExpressions(ocode, "continue", 0);

  while (!producers.empty()) {
    SmartPointer<Producer> producer = producers.back();
    if (!producer.isInstance<Loop>())
      THROWS("Break outside loop or OCode number mismatch");

    if (producer.cast<Loop>()->getNumber() != ocode->getNumber()) {
      producer.cast<Loop>()->continueLoop();
      break;
    }

    producers.pop_back();
  }
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
  loop.number = ocode->getNumber();
  loop.program = new Program;
  loop.end = "endrepeat";
  const OCode::expressions_t &expressions = ocode->getExpressions();
  loop.repeat = expressions.empty() ? 0 : expressions[0]->eval(*this);
}


void OCodeInterpreter::doEndRepeat(OCode *ocode) {
  checkExpressions(ocode, "endrepeat", 0);

  push(new RepeatLoop(ocode->getNumber(), loop.program, loop.repeat));
  loop.program = 0;
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
  if (!loop.program.isNull() &&
      (number != loop.number || keyword != loop.end)) {
    loop.program->push_back(block);
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


void OCodeInterpreter::setReference(address_t addr, double value) {
  if (!addr || 30 < addr || stack.empty())
    GCodeInterpreter::setReference(addr, value);

  else {
    LOG_DEBUG(3, "Set local variable #" << addr << " = " << value);
    stack.back().nums[addr - 1] = value; // Local variable assignment
  }
}


void OCodeInterpreter::setReference(const string &name, double value) {
  if (stack.empty() || name[0] == '_')
    GCodeInterpreter::setReference(name, value);

  else {
    LOG_DEBUG(3, "Set local variable #<" << name << "> = " << value);
    stack.back().names[name] = value; // Local variable assignment
  }
}


double OCodeInterpreter::lookupReference(address_t addr) {
  if (!addr || 30 < addr || stack.empty())
    return GCodeInterpreter::lookupReference(addr);

  // Local variable reference
  return stack.back().nums[addr - 1];
}


double OCodeInterpreter::lookupReference(const string &name) {
  if (name[0] != '_' && !stack.empty()) {
    auto &nameMap = stack.back().names;
    auto it = nameMap.find(name);
    if (it != nameMap.end()) return it->second;
    THROWS("Local reference to '" << name << "' not found");
  }

  return GCodeInterpreter::lookupReference(name);
}
