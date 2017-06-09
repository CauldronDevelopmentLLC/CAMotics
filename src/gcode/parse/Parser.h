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

#pragma once

#include <gcode/Processor.h>
#include <gcode/NullInterrupter.h>

#include <gcode/ast/Block.h>
#include <gcode/ast/Comment.h>
#include <gcode/ast/Word.h>
#include <gcode/ast/Assign.h>
#include <gcode/ast/OCode.h>
#include <gcode/ast/Number.h>
#include <gcode/ast/FunctionCall.h>

#include <cbang/SmartPointer.h>
#include <cbang/io/InputSource.h>

#include <istream>
#include <string>


namespace GCode {
  class Tokenizer;

  class Parser {
    cb::SmartPointer<Interrupter> interrupter;
    unsigned errors;

  public:
    Parser(const cb::SmartPointer<Interrupter> &interrupter =
           new NullInterrupter) :
      interrupter(interrupter), errors(0) {}

    void resetErrorCount() {errors = 0;}
    unsigned getErrorCount() {return errors;}

    void parse(Tokenizer &tokenizer, Processor &processor,
               unsigned maxErrors = 32);
    void parse(const cb::InputSource &source, Processor &processor,
               unsigned maxErrors = 32);

    bool parseOne(Tokenizer &tokenizer, Processor &processor);

    cb::SmartPointer<Block> block(Tokenizer &tokenizer);

    cb::SmartPointer<Comment> comment(Tokenizer &tokenizer);
    cb::SmartPointer<Word> word(Tokenizer &tokenizer);
    cb::SmartPointer<Assign> assign(Tokenizer &tokenizer);
    cb::SmartPointer<OCode> ocode(Tokenizer &tokenizer);

    cb::SmartPointer<Entity> numberRefOrExpr(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> expression(Tokenizer &tokenizer);

    cb::SmartPointer<Entity> boolOp(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> compareOp(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> addOp(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> mulOp(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> expOp(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> unaryOp(Tokenizer &tokenizer);

    cb::SmartPointer<Entity> primary(Tokenizer &tokenizer);

    cb::SmartPointer<Entity> quotedExpr(Tokenizer &tokenizer);
    cb::SmartPointer<FunctionCall> functionCall(Tokenizer &tokenizer);
    cb::SmartPointer<Number> number(Tokenizer &tokenizer);
    cb::SmartPointer<Entity> reference(Tokenizer &tokenizer);
  };
}
