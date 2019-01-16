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

/*
  This file was derived from JSEdit from the Ofi Labs X2 project.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>
  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GCodeHighlighter.h"

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


GCodeHighlighter::GCodeHighlighter(QTextDocument *parent) :
  Highlighter(parent) {

  ops << "AND";
  ops << "OR";
  ops << "XOR";
  ops << "EQ";
  ops << "NE";
  ops << "GT";
  ops << "GE";
  ops << "LT";
  ops << "LE";
  ops << "MOD";

  ocodes << "SUB";
  ocodes << "ENDSUB";
  ocodes << "CALL";
  ocodes << "RETURN";
  ocodes << "DO";
  ocodes << "WHILE";
  ocodes << "ENDWHILE";
  ocodes << "BREAK";
  ocodes << "CONTINUE";
  ocodes << "IF";
  ocodes << "ELSE";
  ocodes << "ENDIF";
  ocodes << "REPEAT";
  ocodes << "ENDREPEAT";

  functions << "ABS";
  functions << "ACOS";
  functions << "ASIN";
  functions << "COS";
  functions << "EXP";
  functions << "FIX";
  functions << "FUP";
  functions << "ROUND";
  functions << "LN";
  functions << "SIN";
  functions << "SQRT";
  functions << "TAN";
  functions << "ATAN";

  colors[ColorComponent::GCodeLineNumber] = QColor("#ffff00");
  colors[ColorComponent::Reference] = QColor("#98fb98");
  colors[ColorComponent::Function] = QColor("#87cefa");
}


void GCodeHighlighter::setFormat(const Token &start, const Token &end,
                                 ColorComponent cc) {
  int sCol = start.getLocation().getStart().getCol();
  int eCol = end.getLocation().getEnd().getCol();
  setFormat(sCol, eCol - sCol, colors[cc]);
}


void GCodeHighlighter::setFormat(const Token &token, ColorComponent cc) {
  setFormat(token, token, cc);
}


void GCodeHighlighter::highlightBlock(const QString &text) {
  try {
    QByteArray array = text.toUtf8();
    GCode::Tokenizer tokenizer(InputSource(array.data(), array.length()));

    // Deleted
    if (tokenizer.consume(DIV_TOKEN))
      setFormat(0, text.length(), colors[ColorComponent::Comment]);

    else {
      // Line Number
      if (tokenizer.isID("N")) {
        Token start = tokenizer.peek();
        tokenizer.advance();
        Token num = tokenizer.peek();
        if (num.getType() == NUMBER_TOKEN) {
          setFormat(start, num, ColorComponent::GCodeLineNumber);
          tokenizer.advance();
        }
      }

      // O-Code
      if (tokenizer.isID("O")) ocode(tokenizer);

      while (tokenizer.hasMore()) {
        switch (tokenizer.getType()) {
        case EOL_TOKEN: break; // End of block

        case COMMENT_TOKEN:
        case PAREN_COMMENT_TOKEN:
          comment(tokenizer);
          break;

        case POUND_TOKEN:
          assign(tokenizer);
          break;

        default:
          if (tokenizer.isType(ID_TOKEN)) word(tokenizer);
          else tokenizer.advance();
          break;
        }
      }
    }

    // Call base class
    Highlighter::highlightBlock(text);

  } catch (const Exception &e) {
    LOG_DEBUG(3, "Highlighting: " << e);
  }
}

void GCodeHighlighter::comment(GCode::Tokenizer &tokenizer) {
  setFormat(tokenizer.advance(), ColorComponent::Comment);
}


void GCodeHighlighter::word(GCode::Tokenizer &tokenizer) {
  tokenizer.advance();
  numberRefOrExpr(tokenizer);
}


void GCodeHighlighter::assign(GCode::Tokenizer &tokenizer) {
  reference(tokenizer);
  tokenizer.consume(ASSIGN_TOKEN);
  expression(tokenizer);
}


void GCodeHighlighter::ocode(GCode::Tokenizer &tokenizer) {
  Token start = tokenizer.peek();
  tokenizer.advance();

  if (tokenizer.isType(OANGLE_TOKEN)) {
    // TODO Color
    tokenizer.advance();
    tokenizer.consume(ID_TOKEN);
    tokenizer.consume(CANGLE_TOKEN);

  } else numberRefOrExpr(tokenizer);

  // Keyword
  if (!tokenizer.isType(ID_TOKEN)) return;
  Token token = tokenizer.advance();
  if (ocodes.contains
      (QString::fromUtf8(String::toUpper(token.getValue()).c_str())))
    setFormat(token, ColorComponent::Keyword);

  // Expressions
  while (tokenizer.isType(OBRACKET_TOKEN))
    quotedExpr(tokenizer);
}


void GCodeHighlighter::numberRefOrExpr(GCode::Tokenizer &tokenizer) {
  switch (tokenizer.getType()) {
  case POUND_TOKEN: return reference(tokenizer);
  case OBRACKET_TOKEN: return quotedExpr(tokenizer);
  case NUMBER_TOKEN: return number(tokenizer);
  case ADD_TOKEN:
  case SUB_TOKEN: return unaryOp(tokenizer);
  default: break;
  }
}


void GCodeHighlighter::expression(GCode::Tokenizer &tokenizer) {
  primary(tokenizer);

  switch (tokenizer.getType()) {
  case ID_TOKEN: {
    string id = String::toUpper(tokenizer.getValue());
    if (!ops.contains(QString::fromUtf8(id.c_str()))) break;
    setFormat(tokenizer.peek(), ColorComponent::Keyword);
  }

  case ADD_TOKEN:
  case SUB_TOKEN:
  case MUL_TOKEN:
  case DIV_TOKEN:
  case EXP_TOKEN:
    tokenizer.advance();
    expression(tokenizer);

  default: break;
  }
}


void GCodeHighlighter::unaryOp(GCode::Tokenizer &tokenizer) {
  tokenizer.advance();
  numberRefOrExpr(tokenizer);
}


void GCodeHighlighter::primary(GCode::Tokenizer &tokenizer) {
  switch (tokenizer.getType()) {
  case ID_TOKEN: return functionCall(tokenizer);
  default: return numberRefOrExpr(tokenizer);
  }
}


void GCodeHighlighter::quotedExpr(GCode::Tokenizer &tokenizer) {
  tokenizer.advance();
  expression(tokenizer);
  tokenizer.consume(CBRACKET_TOKEN);
}


void GCodeHighlighter::functionCall(GCode::Tokenizer &tokenizer) {
  Token func = tokenizer.advance();
  string name = String::toUpper(func.getValue());
  if (functions.contains(QString::fromUtf8(name.c_str())))
    setFormat(func, ColorComponent::Function);

  quotedExpr(tokenizer);

  // Special case
  if (name == "ATAN" && tokenizer.consume(DIV_TOKEN))
    quotedExpr(tokenizer);
}


void GCodeHighlighter::number(GCode::Tokenizer &tokenizer) {
  setFormat(tokenizer.advance(), ColorComponent::Number);
}


void GCodeHighlighter::reference(GCode::Tokenizer &tokenizer) {
  Token pound = tokenizer.advance();

  if (tokenizer.consume(OANGLE_TOKEN)) {
    if (tokenizer.consume(ID_TOKEN) &&
        tokenizer.isType(CANGLE_TOKEN)) {
      setFormat(pound, tokenizer.advance(), ColorComponent::Reference);
    }

  } else if (tokenizer.isType(NUMBER_TOKEN))
    setFormat(pound, tokenizer.advance(), ColorComponent::Reference);

  else numberRefOrExpr(tokenizer);
}
