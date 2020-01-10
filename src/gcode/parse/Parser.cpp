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

#include "Parser.h"

#include <gcode/ast/UnaryOp.h>
#include <gcode/ast/BinaryOp.h>
#include <gcode/ast/QuotedExpr.h>
#include <gcode/ast/Reference.h>
#include <gcode/ast/NamedReference.h>

#include <cbang/String.h>
#include <cbang/Exception.h>
#include <cbang/log/Logger.h>
#include <cbang/parse/ParseScope.h>

#include <cctype>

using namespace std;
using namespace cb;
using namespace GCode;


namespace {
  string tokenDescription(GCode::Token t) {
    typedef GCode::TokenType T;

    switch (t.getType()) {
    case T::EOF_TOKEN:           return "End of input";
    case T::COMMENT_TOKEN:       return "Comment";
    case T::PAREN_COMMENT_TOKEN: return "'('";
    case T::NUMBER_TOKEN:        return "Number '" + t.getValue() + "'";
    case T::ID_TOKEN:            return "ID '" + t.getValue() + "'";
    case T::EXP_TOKEN:           return "'**'";
    case T::MUL_TOKEN:           return "'*'";
    case T::DIV_TOKEN:           return "'/'";
    case T::ADD_TOKEN:           return "'+'";
    case T::SUB_TOKEN:           return "'-'";
    case T::OBRACKET_TOKEN:      return "'['";
    case T::CBRACKET_TOKEN:      return "']'";
    case T::OANGLE_TOKEN:        return "'<'";
    case T::CANGLE_TOKEN:        return "'>'";
    case T::ASSIGN_TOKEN:        return "'='";
    case T::POUND_TOKEN:         return "'#'";
    case T::DOT_TOKEN:           return "'.'";
    case T::EOL_TOKEN:           return "End of line";
    }

    return t.getType().toString();
  }
}


Parser::Parser(const InputSource &source) :
  tokenizer(new GCode::Tokenizer(source)) {}


unsigned Parser::parse(Processor &processor, unsigned maxErrors) {
  unsigned errors = 0;

  while (tokenizer->hasMore()) {
    try {
      processor(next());

    } catch (const Exception &e) {
      LOG_ERROR(e);
      if (maxErrors < ++errors) THROW("Too many errors aborting");
    }
  }

  return errors;
}


SmartPointer<Block> Parser::next() {
  try {
    return block();

  } catch (const Exception &e) {
    LOG_DEBUG(3, e);
    throw Exception(e.getMessage(), tokenizer->getLocation().getStart());
  }
}


SmartPointer<Block> Parser::block() {
  ParseScope scope(tokenizer->getScanner());

  // Deleted
  bool deleted = false;
  if (tokenizer->consume(TokenType::DIV_TOKEN)) deleted = true;

  // Line number
  int line = -1;
  if (tokenizer->isID("N")) {
    tokenizer->advance();
    Token num = tokenizer->match(TokenType::NUMBER_TOKEN);
    line = String::parseU32(num.getValue());
  }

  // Children
  vector<SmartPointer<Entity> > children;

  // O-Code
  if (tokenizer->isID("O")) children.push_back(ocode());

  while (tokenizer->hasMore()) {
    switch (tokenizer->getType()) {
    case TokenType::EOL_TOKEN: break; // End of block

    case TokenType::COMMENT_TOKEN:
    case TokenType::PAREN_COMMENT_TOKEN:
      children.push_back(comment());
      break;

    case TokenType::POUND_TOKEN:
      children.push_back(assign());
      break;

    default:
      if (!tokenizer->isType(TokenType::ID_TOKEN))
        THROW("Expected word or assignment, found "
              << tokenDescription(tokenizer->advance()));

      children.push_back(word());
      break;
    }

    if (tokenizer->getType() == TokenType::EOL_TOKEN) {
      tokenizer->advance();
      break;
    }
  }

  return scope.set(new Block(deleted, line, children));
}


SmartPointer<Comment> Parser::comment() {
  ParseScope scope(tokenizer->getScanner());

  Token token;
  bool paren = tokenizer->getType() == TokenType::PAREN_COMMENT_TOKEN;

  if (paren) token = tokenizer->advance();
  else token = tokenizer->match(TokenType::COMMENT_TOKEN);

  return scope.set(new Comment(token.getValue(), paren));
}


SmartPointer<Word> Parser::word() {
  ParseScope scope(tokenizer->getScanner());

  string name = tokenizer->match(TokenType::ID_TOKEN).getValue();
  if (name.length() != 1) THROW("Invalid word '" << name << "'");

  return scope.set(new Word(toupper(name[0]), numberRefOrExpr()));
}


SmartPointer<Assign> Parser::assign() {
  ParseScope scope(tokenizer->getScanner());

  SmartPointer<Entity> ref = reference();
  tokenizer->match(TokenType::ASSIGN_TOKEN);

  return scope.set(new Assign(ref, expression()));
}


SmartPointer<OCode> Parser::ocode() {
  ParseScope scope(tokenizer->getScanner());

  tokenizer->match(TokenType::ID_TOKEN); // The 'O'

  SmartPointer<OCode> ocode;

  if (tokenizer->isType(TokenType::OANGLE_TOKEN)) {
    tokenizer->match(TokenType::OANGLE_TOKEN);
    string name = tokenizer->match(TokenType::ID_TOKEN).getValue();
    tokenizer->match(TokenType::CANGLE_TOKEN);
    string keyword = tokenizer->match(TokenType::ID_TOKEN).getValue();

    ocode = new OCode(String::toLower(name), keyword);

  } else {
    SmartPointer<Entity> numExpr = numberRefOrExpr();
    string keyword;

    // Some postprocesors output an O-Code wo/ a keyword as a program number.
    if (tokenizer->isType(TokenType::ID_TOKEN))
      keyword = tokenizer->match(TokenType::ID_TOKEN).getValue();

    ocode = new OCode(numExpr, keyword);
  }

  while (tokenizer->isType(TokenType::OBRACKET_TOKEN))
    ocode->addExpression(quotedExpr());

  scope.set(ocode->getLocation());
  return ocode;
}


SmartPointer<Entity> Parser::numberRefOrExpr() {
  switch (tokenizer->getType()) {
  case TokenType::POUND_TOKEN:    return reference();
  case TokenType::OBRACKET_TOKEN: return quotedExpr();
  case TokenType::NUMBER_TOKEN:   return number();
  case TokenType::ADD_TOKEN:
  case TokenType::SUB_TOKEN:      return unaryOp();
  default:
    THROW("Expected number, reference, or bracked expression, found "
          << tokenDescription(tokenizer->advance()));
  }
}


SmartPointer<Entity> Parser::expression() {
  return boolOp();
}


SmartPointer<Entity> Parser::boolOp() {
  SmartPointer<Entity> entity = compareOp();

  while (true) {
    if (tokenizer->getType() == TokenType::ID_TOKEN) {
      Operator op;

      string id = String::toUpper(tokenizer->getValue());
      if      (id == "AND") op = Operator::AND_OP;
      else if (id == "OR")  op = Operator::OR_OP;
      else if (id == "XOR") op = Operator::XOR_OP;

      if (op != Operator::NO_OP) {
        tokenizer->advance();
        entity = new BinaryOp(op, entity, compareOp());
        continue;
      }
    }
    break;
  }

  return entity;
}


SmartPointer<Entity> Parser::compareOp() {
  SmartPointer<Entity> entity = addOp();

  while (true) {
    if (tokenizer->getType() == TokenType::ID_TOKEN) {
      Operator op;

      string id = String::toUpper(tokenizer->getValue());
      if      (id == "EQ") op = Operator::EQ_OP;
      else if (id == "NE") op = Operator::NE_OP;
      else if (id == "GT") op = Operator::GT_OP;
      else if (id == "GE") op = Operator::GE_OP;
      else if (id == "LT") op = Operator::LT_OP;
      else if (id == "LE") op = Operator::LE_OP;

      if (op != Operator::NO_OP) {
        tokenizer->advance();
        entity = new BinaryOp(op, entity, addOp());
        continue;
      }
    }
    break;
  }

  return entity;
}


SmartPointer<Entity> Parser::addOp() {
  SmartPointer<Entity> entity = mulOp();

  while (true) {
    Operator op;
    switch(tokenizer->getType()) {
    case TokenType::ADD_TOKEN: op = Operator::ADD_OP; break;
    case TokenType::SUB_TOKEN: op = Operator::SUB_OP; break;
    default: break;
    }

    if (op == Operator::NO_OP) break;

    tokenizer->advance();
    entity = new BinaryOp(op, entity, mulOp());
  }

  return entity;
}


SmartPointer<Entity> Parser::mulOp() {
  SmartPointer<Entity> entity = expOp();

  while (true) {
    Operator op;
    switch(tokenizer->getType()) {
    case TokenType::MUL_TOKEN: op = Operator::MUL_OP; break;
    case TokenType::DIV_TOKEN: op = Operator::DIV_OP; break;
    case TokenType::ID_TOKEN:
      if (String::toUpper(tokenizer->getValue()) == "MOD")
        op = Operator::MOD_OP;
      break;
    default: break;
    }

    if (op == Operator::NO_OP) break;

    tokenizer->advance();
    entity = new BinaryOp(op, entity, expOp());
  }

  return entity;
}


SmartPointer<Entity> Parser::expOp() {
  SmartPointer<Entity> entity = primary();

  while (true)
    if (tokenizer->consume(TokenType::EXP_TOKEN))
      entity = new BinaryOp(Operator::EXP_OP, entity, primary());
    else break;

  return entity;
}


SmartPointer<Entity> Parser::unaryOp() {
  Operator op;
  switch(tokenizer->getType()) {
  case TokenType::ADD_TOKEN: op = Operator::ADD_OP; break;
  case TokenType::SUB_TOKEN: op = Operator::SUB_OP; break;
  default:
    THROW("Expected unary - or + operator, found "
           << tokenizer->advance().getType());
  }

  tokenizer->advance();

  return new UnaryOp(op, numberRefOrExpr());
}


SmartPointer<Entity> Parser::primary() {
  switch (tokenizer->getType()) {
  case TokenType::ID_TOKEN: return functionCall();
  default: return numberRefOrExpr();
  }
}


SmartPointer<Entity> Parser::quotedExpr() {
  ParseScope scope(tokenizer->getScanner());

  tokenizer->match(TokenType::OBRACKET_TOKEN);
  SmartPointer<Entity> expr = expression();
  tokenizer->match(TokenType::CBRACKET_TOKEN);

  return scope.set(new QuotedExpr(expr));
}


SmartPointer<FunctionCall>
Parser::functionCall() {
  ParseScope scope(tokenizer->getScanner());
  string name = tokenizer->match(TokenType::ID_TOKEN).getValue();
  SmartPointer<Entity> arg1 = quotedExpr();
  SmartPointer<Entity> arg2;

  // Special case
  if (String::toUpper(name) == "ATAN" &&
      tokenizer->consume(TokenType::DIV_TOKEN))
    arg2 = quotedExpr();

  return scope.set(new FunctionCall(name, arg1, arg2));
}


SmartPointer<Number> Parser::number() {
  ParseScope scope(tokenizer->getScanner());
  double value =
    String::parseDouble(tokenizer->match(TokenType::NUMBER_TOKEN).getValue());

  return scope.set(new Number(value));
}


SmartPointer<Entity> Parser::reference() {
  ParseScope scope(tokenizer->getScanner());

  tokenizer->match(TokenType::POUND_TOKEN);

  if (tokenizer->consume(TokenType::OANGLE_TOKEN)) {
    string id;

    while (tokenizer->getType() != TokenType::CANGLE_TOKEN)
      id += tokenizer->advance().getValue();

    tokenizer->match(TokenType::CANGLE_TOKEN);

    return scope.set(new NamedReference(id));
  }

  return scope.set(new Reference(numberRefOrExpr()));
}
