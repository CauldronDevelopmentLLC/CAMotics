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

#ifndef OPENSCAM_PARSER_H
#define OPENSCAM_PARSER_H

#include "Processor.h"

#include "ast/Block.h"
#include "ast/Comment.h"
#include "ast/Word.h"
#include "ast/Assign.h"
#include "ast/OCode.h"
#include "ast/Number.h"
#include "ast/FunctionCall.h"

#include <openscam/Task.h>

#include <cbang/SmartPointer.h>
#include <cbang/io/InputSource.h>

#include <istream>
#include <string>


namespace OpenSCAM {
  class Tokenizer;

  class Parser {
    cb::SmartPointer<Task> task;

  public:
    Parser(const cb::SmartPointer<Task> &task = new Task) : task(task) {}

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

#endif // OPENSCAM_PARSER_H

