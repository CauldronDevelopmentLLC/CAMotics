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

#include "Tokenizer.h"

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cctype>

using namespace std;
using namespace GCode;


namespace {
  int whiteSpace[] = {0x20, 0x09, 0x0d, 0xfeff, 0};
}


bool Tokenizer::isID(const string &id) const {
  return isType(ID_TOKEN) &&
    cb::String::toUpper(id) == cb::String::toUpper(getValue());
}


void Tokenizer::comment() {
  scanner->match(';');
  current.setType(COMMENT_TOKEN);

  string value;
  while (scanner->hasMore() && scanner->peek() != '\n') {
    if (scanner->peek() != '\r') value.append(1, scanner->peek());
    scanner->advance();
  }

  current.setValue(value);
}


void Tokenizer::parenComment() {
  scanner->match('(');
  current.setType(PAREN_COMMENT_TOKEN);

  string value;
  while (scanner->hasMore() && scanner->peek() != ')') {
    value.append(1, scanner->peek());
    scanner->advance();
  }

  scanner->match(')');

  current.setValue(value);
}


void Tokenizer::number(bool positive) {
  string value;
  bool foundDot = false;
  char c = scanner->peek();

  do {
    if (c == '.') {
      if (foundDot) break; // Already found decimal point
      foundDot = true;
    }

    value.append(string(1, c));

    scanner->advance();
    scanner->skipWhiteSpace(whiteSpace);
    if (!scanner->hasMore()) break;
    c = scanner->peek();

  } while (isdigit(c) || (!foundDot && c == '.'));


  if (foundDot && value.length() == 1)
    return current.set(DOT_TOKEN, '.');

  if (!positive) value = string("-") + value;

  current.set(NUMBER_TOKEN, value);
}


void Tokenizer::id() {
  string value;

  char c = scanner->peek();
  while (isalpha(c) || c == '_') {
    value.append(string(1, c));

    scanner->advance();
    if (!scanner->hasMore()) break;
    c = scanner->peek();
  }

  current.set(ID_TOKEN, value);
}


void Tokenizer::next() {
  scanner->skipWhiteSpace(whiteSpace);

  cb::FileLocation start = scanner->getLocation();

  if (!scanner->hasMore()) {
    current.set(EOF_TOKEN, "");
    return;
  }

  bool needAdvance = true;
  int c = scanner->peek();
  switch (c) {
  case 0: current.set(EOF_TOKEN, ""); return;

  case '%': break; // Ignore program delimiter
  case ';': comment(); needAdvance = false; break;
  case '(': parenComment(); needAdvance = false; break;

  case '.':
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    number();
    needAdvance = false;
    break;

  case '*':
    scanner->advance();
    if (scanner->peek() == '*') current.set(EXP_TOKEN, "**");
    else {
      current.set(MUL_TOKEN, c);
      needAdvance = false;
    }
    break;

  case '+': current.set(ADD_TOKEN, c); break;
  case '-': current.set(SUB_TOKEN, c); break;
  case '/': current.set(DIV_TOKEN, c); break;
  case '[': current.set(OBRACKET_TOKEN, c); break;
  case ']': current.set(CBRACKET_TOKEN, c); break;
  case '<': current.set(OANGLE_TOKEN, c); break;
  case '>': current.set(CANGLE_TOKEN, c); break;
  case '=': current.set(ASSIGN_TOKEN, c); break;
  case '#': current.set(POUND_TOKEN, c); break;
  case '\n': current.set(EOL_TOKEN, c); break;

  default:
    if (isalpha(c) || c == '_') {
      id();
      needAdvance = false;

    } else {
      scanner->advance();
      THROW("Invalid character: '" << cb::String::escapeC(c) << "'");
    }
  }

  if (needAdvance) scanner->advance();

  current.getLocation() = cb::LocationRange(start, scanner->getLocation());
}
