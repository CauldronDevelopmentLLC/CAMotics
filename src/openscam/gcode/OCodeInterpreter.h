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

#ifndef OPENSCAM_OCODE_INTERPRETER_H
#define OPENSCAM_OCODE_INTERPRETER_H

#include "GCodeInterpreter.h"

#include <cbang/SmartPointer.h>

#include <map>
#include <set>
#include <vector>


namespace OpenSCAM {
  class Program;

  class OCodeInterpreter : public GCodeInterpreter {
    typedef std::map<unsigned, cb::SmartPointer<Program> > subroutines_t;
    subroutines_t subroutines;
    typedef std::map<std::string, cb::SmartPointer<Program> >
    named_subroutines_t;
    named_subroutines_t namedSubroutines;
    cb::SmartPointer<Program> subroutine;
    unsigned subroutineNumber;
    std::string subroutineName;

    // Variable call stack
    typedef std::vector<std::vector<double> > stack_t;
    stack_t stack;
    typedef std::map<std::string, double> name_map_t;
    typedef std::vector<name_map_t> name_stack_t;
    name_stack_t nameStack;

    std::vector<unsigned> conditions;
    bool condition;

    unsigned loopNumber;
    cb::SmartPointer<Program> loop;
    std::string loopEnd;
    cb::SmartPointer<Entity> loopExpr;
    unsigned repeat;

    std::set<std::string> loadedFiles;

  public:
    OCodeInterpreter(Controller &controller);
    virtual ~OCodeInterpreter();

    void checkExpressions(OCode *ocode, const char *name, unsigned count);
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
    void doEndIf(OCode *ocode);
    void doRepeat(OCode *ocode);
    void doEndRepeat(OCode *ocode);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

    // From GCodeInterpreter
    void setReference(unsigned num, double value);
    void setReference(const std::string &name, double value);

    // From Evaluator
    double lookupReference(unsigned num);
    double lookupReference(const std::string &name);
  };
}

#endif // OPENSCAM_OCODE_INTERPRETER_H

