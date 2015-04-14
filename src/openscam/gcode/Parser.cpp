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

#include "Parser.h"

#include "Tokenizer.h"

#include "ast/UnaryOp.h"
#include "ast/BinaryOp.h"
#include "ast/QuotedExpr.h"
#include "ast/Reference.h"
#include "ast/NamedReference.h"

#include <cbang/String.h>
#include <cbang/Exception.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/parse/ParseScope.h>

#include <fstream>
#include <cctype>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void Parser::parse(OpenSCAM::Tokenizer &tokenizer, Processor &processor) {
  while (!task->shouldQuit() && parseOne(tokenizer, processor)) continue;
}


void Parser::parse(const InputSource &source, Processor &processor) {
  Scanner scanner(source);
  OpenSCAM::Tokenizer tokenizer(scanner);

  parse(tokenizer, processor);
}


bool Parser::parseOne(OpenSCAM::Tokenizer &tokenizer, Processor &processor) {
  try {
    if (!tokenizer.hasMore()) return false;
    processor(block(tokenizer));
    return true;
  } catch (const Exception &e) {
    THROWCS("Exception @" << tokenizer.getLocation(), e);
  }
}


SmartPointer<Block> Parser::block(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  // Deleted
  bool deleted = false;
  if (tokenizer.consume(TokenType::DIV_TOKEN)) deleted = true;

  // Line number
  int line = -1;
  if (tokenizer.isID("N")) {
    tokenizer.advance();
    Token num = tokenizer.match(TokenType::NUMBER_TOKEN);
    line = String::parseU32(num.getValue());
  }

  // Children
  std::vector<SmartPointer<Entity> > children;

  // O-Code
  if (tokenizer.isID("O")) children.push_back(ocode(tokenizer));

  while (tokenizer.hasMore()) {
    switch (tokenizer.getType()) {
    case TokenType::EOL_TOKEN: break; // End of block

    case TokenType::COMMENT_TOKEN:
    case TokenType::PAREN_COMMENT_TOKEN:
      children.push_back(comment(tokenizer));
      break;

    case TokenType::POUND_TOKEN:
      children.push_back(assign(tokenizer));
      break;

    default:
      if (!tokenizer.isType(TokenType::ID_TOKEN))
        THROWS("Expected word or assignment, found " << tokenizer.getType());

      children.push_back(word(tokenizer));
      break;
    }

    if (tokenizer.getType() == TokenType::EOL_TOKEN) {
      tokenizer.advance();
      break;
    }
  }

  return scope.set(new Block(deleted, line, children));
}


SmartPointer<Comment> Parser::comment(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  Token token;
  bool paren = tokenizer.getType() == TokenType::PAREN_COMMENT_TOKEN;

  if (paren) token = tokenizer.advance();
  else token = tokenizer.match(TokenType::COMMENT_TOKEN);

  return scope.set(new Comment(token.getValue(), paren));
}


SmartPointer<Word> Parser::word(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  string name = tokenizer.match(TokenType::ID_TOKEN).getValue();
  if (name.length() != 1) THROWS("Invalid word '" << name << "'");

  return scope.set(new Word(toupper(name[0]), numberRefOrExpr(tokenizer)));
}


SmartPointer<Assign> Parser::assign(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  SmartPointer<Entity> ref = reference(tokenizer);
  tokenizer.match(TokenType::ASSIGN_TOKEN);

  return scope.set(new Assign(ref, expression(tokenizer)));
}


SmartPointer<OCode> Parser::ocode(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  tokenizer.match(TokenType::ID_TOKEN); // The 'O'

  SmartPointer<OCode> ocode;

  if (tokenizer.isType(TokenType::OANGLE_TOKEN)) {
    tokenizer.match(TokenType::OANGLE_TOKEN);
    string name = tokenizer.match(TokenType::ID_TOKEN).getValue();
    tokenizer.match(TokenType::CANGLE_TOKEN);
    string keyword = tokenizer.match(TokenType::ID_TOKEN).getValue();

    ocode = new OCode(String::toLower(name), keyword);

  } else {
    SmartPointer<Entity> numExpr = numberRefOrExpr(tokenizer);
    string keyword = tokenizer.match(TokenType::ID_TOKEN).getValue();

    ocode = new OCode(numExpr, keyword);
  }

  while (tokenizer.isType(TokenType::OBRACKET_TOKEN))
    ocode->addExpression(quotedExpr(tokenizer));

  scope.set(ocode->getLocation());
  return ocode;
}


SmartPointer<Entity> Parser::numberRefOrExpr(OpenSCAM::Tokenizer &tokenizer) {
  switch (tokenizer.getType()) {
  case TokenType::POUND_TOKEN: return reference(tokenizer);
  case TokenType::OBRACKET_TOKEN: return quotedExpr(tokenizer);
  case TokenType::NUMBER_TOKEN: return number(tokenizer);
  case TokenType::ADD_TOKEN:
  case TokenType::SUB_TOKEN: return unaryOp(tokenizer);
  default: THROW("Expected number, reference, or bracked expression");
  }
}


SmartPointer<Entity> Parser::expression(OpenSCAM::Tokenizer &tokenizer) {
  return boolOp(tokenizer);
}


SmartPointer<Entity> Parser::boolOp(OpenSCAM::Tokenizer &tokenizer) {
  SmartPointer<Entity> entity = compareOp(tokenizer);

  while (true) {
    if (tokenizer.getType() == TokenType::ID_TOKEN) {
      Operator op;

      string id = String::toUpper(tokenizer.getValue());
      if (id == "AND") op = Operator::AND_OP;
      else if (id == "OR") op = Operator::OR_OP;
      else if (id == "XOR") op = Operator::XOR_OP;

      if (op != Operator::NO_OP) {
        tokenizer.advance();
        entity = new BinaryOp(op, entity, compareOp(tokenizer));
      }
    }
    break;
  }

  return entity;
}


SmartPointer<Entity> Parser::compareOp(OpenSCAM::Tokenizer &tokenizer) {
  SmartPointer<Entity> entity = addOp(tokenizer);

  while (true) {
    if (tokenizer.getType() == TokenType::ID_TOKEN) {
      Operator op;

      string id = String::toUpper(tokenizer.getValue());
      if (id == "EQ") op = Operator::EQ_OP;
      else if (id == "NE") op = Operator::NE_OP;
      else if (id == "GT") op = Operator::GT_OP;
      else if (id == "GE") op = Operator::GE_OP;
      else if (id == "LT") op = Operator::LT_OP;
      else if (id == "LE") op = Operator::LE_OP;

      if (op != Operator::NO_OP) {
        tokenizer.advance();
        entity = new BinaryOp(op, entity, addOp(tokenizer));
        continue;
      }
    }
    break;
  }

  return entity;  
}


SmartPointer<Entity> Parser::addOp(OpenSCAM::Tokenizer &tokenizer) {
  SmartPointer<Entity> entity = mulOp(tokenizer);

  while (true) {
    Operator op;
    switch(tokenizer.getType()) {
    case TokenType::ADD_TOKEN: op = Operator::ADD_OP; break;
    case TokenType::SUB_TOKEN: op = Operator::SUB_OP; break;
    default: break;
    }

    if (op != Operator::NO_OP) {
      tokenizer.advance();
      entity = new BinaryOp(op, entity, mulOp(tokenizer));

    } else break;
  }

  return entity;  
}


SmartPointer<Entity> Parser::mulOp(OpenSCAM::Tokenizer &tokenizer) {
  SmartPointer<Entity> entity = expOp(tokenizer);

  while (true) {
    Operator op;
    switch(tokenizer.getType()) {
    case TokenType::MUL_TOKEN: op = Operator::MUL_OP; break;
    case TokenType::DIV_TOKEN: op = Operator::DIV_OP; break;
    case TokenType::ID_TOKEN:
      if (String::toUpper(tokenizer.getValue()) == "MOD")
        op = Operator::MOD_OP;
      break;
    default: break;
    }

    if (op != Operator::NO_OP) {
      tokenizer.advance();
      entity = new BinaryOp(op, entity, expOp(tokenizer));

    } else break;
  }

  return entity;  
}


SmartPointer<Entity> Parser::expOp(OpenSCAM::Tokenizer &tokenizer) {
  SmartPointer<Entity> entity = primary(tokenizer);

  while (true) {
    if (tokenizer.consume(TokenType::EXP_TOKEN))
      entity = new BinaryOp(Operator::EXP_OP, entity, primary(tokenizer));
    else break;
  }

  return entity;
}


SmartPointer<Entity> Parser::unaryOp(OpenSCAM::Tokenizer &tokenizer) {
  Operator op;
  switch(tokenizer.getType()) {
  case TokenType::ADD_TOKEN: op = Operator::ADD_OP; break;
  case TokenType::SUB_TOKEN: op = Operator::SUB_OP; break;
  default: break;
  }

  if (op == Operator::NO_OP) THROW("Expected unary - or + operator");
  tokenizer.advance();

  return new UnaryOp(op, numberRefOrExpr(tokenizer));
}


SmartPointer<Entity> Parser::primary(OpenSCAM::Tokenizer &tokenizer) {
  switch (tokenizer.getType()) {
  case TokenType::ID_TOKEN: return functionCall(tokenizer);
  default: return numberRefOrExpr(tokenizer);
  }
}


SmartPointer<Entity> Parser::quotedExpr(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  tokenizer.match(TokenType::OBRACKET_TOKEN);
  SmartPointer<Entity> expr = expression(tokenizer);
  tokenizer.match(TokenType::CBRACKET_TOKEN);

  return scope.set(new QuotedExpr(expr));
}


SmartPointer<FunctionCall>
Parser::functionCall(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());
  string name = tokenizer.match(TokenType::ID_TOKEN).getValue();
  SmartPointer<Entity> arg1 = quotedExpr(tokenizer);
  SmartPointer<Entity> arg2;

  // Special case
  if (String::toUpper(name) == "ATAN" &&
      tokenizer.consume(TokenType::DIV_TOKEN))
    arg2 = quotedExpr(tokenizer);

  return scope.set(new FunctionCall(name, arg1, arg2));
}


SmartPointer<Number> Parser::number(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());
  double value =
    String::parseDouble(tokenizer.match(TokenType::NUMBER_TOKEN).getValue());

  return scope.set(new Number(value));
}


SmartPointer<Entity> Parser::reference(OpenSCAM::Tokenizer &tokenizer) {
  ParseScope scope(tokenizer.getScanner());

  tokenizer.match(TokenType::POUND_TOKEN);

  if (tokenizer.consume(TokenType::OANGLE_TOKEN)) {
    string id = tokenizer.match(TokenType::ID_TOKEN).getValue();
    tokenizer.match(TokenType::CANGLE_TOKEN);

    return scope.set(new NamedReference(id));

  } else
    return scope.set(new Reference(numberRefOrExpr(tokenizer)));
}
