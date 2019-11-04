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


#include "GCodeInterpreter.h"
#include "ProducerStack.h"

#include <map>
#include <set>
#include <vector>


namespace GCode {
  class OCodeInterpreter : public GCodeInterpreter, public ProducerStack {
    // Subroutines
    struct {
      std::map<unsigned, cb::SmartPointer<Program> > numbered;
      std::map<std::string, cb::SmartPointer<Program> > named;

      cb::SmartPointer<Program> current;
      unsigned number;
      std::string name;
      std::set<std::string> loadedFiles;
    } sub;

    // Variable call stack
    struct StackEntry {
      std::vector<double> nums;
      std::map<std::string, double> names;

      StackEntry() : nums(30) {}
    };

    std::vector<StackEntry> stack;

    // Conditions
    std::vector<unsigned> conditions;
    bool ifSatisfied;
    bool condition;

    // Loops
    struct {
      unsigned number;
      cb::SmartPointer<Program> program;
      std::string end;
      cb::SmartPointer<Entity> expr;
      unsigned repeat;
    } loop;


  public:
    OCodeInterpreter(Controller &controller);
    ~OCodeInterpreter() {ProducerStack::unwind();}

    const cb::SmartPointer<Program> &
    lookupSubroutine(const std::string &name) const;

    void checkExpressions(OCode *ocode, const char *name, bool expr = false,
                          bool optional = false);
    void upScope();
    void downScope();

    void doSub(OCode *ocode);
    void doEndSub(OCode *ocode);
    void doCall(OCode *ocode);
    void doReturn(OCode *ocode);
    void doDo(OCode *ocode);
    void doWhile(OCode *ocode);
    void doEndWhile(OCode *ocode);
    void doBreak(OCode *ocode);
    void doContinue(OCode *ocode);
    void doIf(OCode *ocode);
    void doElse(OCode *ocode);
    void doElseIf(OCode *ocode);
    void doEndIf(OCode *ocode);
    void doRepeat(OCode *ocode);
    void doEndRepeat(OCode *ocode);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

    // From GCodeInterpreter
    void setReference(address_t addr, double value);
    void setReference(const std::string &name, double value);

    // From Evaluator
    double lookupReference(address_t addr);
    double lookupReference(const std::string &name);
  };
}
