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

#include "Tokenizer.h"

#include <gcode/Processor.h>
#include <gcode/Producer.h>

#include <gcode/ast/Comment.h>
#include <gcode/ast/Word.h>
#include <gcode/ast/Assign.h>
#include <gcode/ast/OCode.h>
#include <gcode/ast/Number.h>
#include <gcode/ast/FunctionCall.h>

#include <cbang/io/InputSource.h>


namespace GCode {
  class Parser : public Producer {
    cb::SmartPointer<Tokenizer> tokenizer;

  public:
    Parser(const cb::SmartPointer<Tokenizer> &tokenizer) :
      tokenizer(tokenizer) {}
    Parser(Tokenizer &tokenizer) :
      tokenizer(cb::SmartPointer<Tokenizer>::Phony(&tokenizer)) {}
    Parser(const cb::InputSource &source);

    unsigned parse(Processor &processor, unsigned maxErrors = 32);

    // From Producer
    bool hasMore() const {return tokenizer->hasMore();}
    cb::SmartPointer<Block> next();

    cb::SmartPointer<Block> block();

    cb::SmartPointer<Comment> comment();
    cb::SmartPointer<Word> word();
    cb::SmartPointer<Assign> assign();
    cb::SmartPointer<OCode> ocode();

    cb::SmartPointer<Entity> numberRefOrExpr();
    cb::SmartPointer<Entity> expression();

    cb::SmartPointer<Entity> boolOp();
    cb::SmartPointer<Entity> compareOp();
    cb::SmartPointer<Entity> addOp();
    cb::SmartPointer<Entity> mulOp();
    cb::SmartPointer<Entity> expOp();
    cb::SmartPointer<Entity> unaryOp();

    cb::SmartPointer<Entity> primary();

    cb::SmartPointer<Entity> quotedExpr();
    cb::SmartPointer<FunctionCall> functionCall();
    cb::SmartPointer<Number> number();
    cb::SmartPointer<Entity> reference();
  };
}
