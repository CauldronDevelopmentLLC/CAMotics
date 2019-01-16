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

#include "TPLHighlighter.h"

#include <cbang/log/Logger.h>

using namespace CAMotics;


TPLHighlighter::TPLHighlighter(QTextDocument *parent) : Highlighter(parent) {
  // https://developer.mozilla.org/en/JavaScript/Reference/Reserved_Words
  // Current
  keywords << "break";
  keywords << "case";
  keywords << "catch";
  keywords << "continue";
  keywords << "default";
  keywords << "delete";
  keywords << "do";
  keywords << "else";
  keywords << "finally";
  keywords << "for";
  keywords << "function";
  keywords << "if";
  keywords << "in";
  keywords << "instanceof";
  keywords << "new";
  keywords << "return";
  keywords << "switch";
  keywords << "this";
  keywords << "throw";
  keywords << "try";
  keywords << "typeof";
  keywords << "var";
  keywords << "void";
  keywords << "while";
  keywords << "with";

  // Future
  keywords << "abstract";
  keywords << "boolean";
  keywords << "byte";
  keywords << "char";
  keywords << "class";
  keywords << "const";
  keywords << "debugger";
  keywords << "double";
  keywords << "enum";
  keywords << "export";
  keywords << "extends";
  keywords << "final";
  keywords << "float";
  keywords << "goto";
  keywords << "implements";
  keywords << "import";
  keywords << "int";
  keywords << "interface";
  keywords << "long";
  keywords << "native";
  keywords << "package";
  keywords << "private";
  keywords << "protected";
  keywords << "public";
  keywords << "short";
  keywords << "static";
  keywords << "super";
  keywords << "synchronized";
  keywords << "throws";
  keywords << "transient";
  keywords << "volatile";

  // Constants
  keywords << "true";
  keywords << "false";
  keywords << "null";
}


void TPLHighlighter::highlightBlock(const QString &text) {
  // Parsing state
  enum {
    Start,
    Number,
    Identifier,
    String,
    Comment,
    Regex,
  };

  QList<int> bracketPositions;

  int blockState = previousBlockState();
  int bracketLevel = blockState >> 8;
  int state = blockState & 0xf;
  int precedingState = (blockState >> 4) & 0xf;

  if (blockState < 0) {
    bracketLevel = 0;
    precedingState = state = Start;
  }

  int start = 0;
  int i = 0;
  int nextState = Start;
  QChar lastCh = QChar();

  while (i <= text.length()) {
    QChar ch = (i < text.length()) ? text.at(i) : QChar();
    QChar next = (i < text.length() - 1) ? text.at(i + 1) : QChar();
    nextState = state;

    switch (state) {
    case Start:
      start = i;

      if (ch.isSpace()) i++;

      else if (ch.isDigit()) {
        i++;
        nextState = Number;

      } else if (ch.isLetter() || ch == '_') {
        i++;
        nextState = Identifier;

      } else if (ch == '\'' || ch == '\"') {
        i++;
        nextState = String;

      } else if (ch == '/' && next == '*') {
        i++;
        i++;
        nextState = Comment;

      } else if (ch == '/' && next == '/') {
        i = text.length();
        setFormat(start, text.length(), colors[ColorComponent::Comment]);

      } else if (ch == '/' && next != '*') {
        i++;
        nextState = Regex;

        switch (precedingState) {
        case Number:
        case Identifier:
        case String:
          setFormat(start, 1, colors[ColorComponent::Operator]);
          nextState = Start;
          break;

        default:
          if (QString(")]}").contains(lastCh)) {
            setFormat(start, 1, colors[ColorComponent::Operator]);
            nextState = Start;
          }
          break;
        }

      } else {
        if (!QString("()[]{}").contains(ch))
          setFormat(start, 1, colors[ColorComponent::Operator]);

        if (ch =='{' || ch == '}') {
          bracketPositions += i;
          if (ch == '{') bracketLevel++;
          else bracketLevel--;
        }

        i++;
        nextState = Start;
      }
      break;

    case Number:
      if (ch.isSpace() || !ch.isDigit()) {
        setFormat(start, i - start, colors[ColorComponent::Number]);
        nextState = Start;

      } else i++;
      break;

    case Identifier:
      if (ch.isSpace() || !(ch.isDigit() || ch.isLetter() || ch == '_')) {
        QString token = text.mid(start, i - start).trimmed();

        if (keywords.contains(token))
          setFormat(start, i - start, colors[ColorComponent::Keyword]);

        else if (builtins.contains(token))
          setFormat(start, i - start, colors[ColorComponent::BuiltIn]);

        nextState = Start;

      } else i++;
      break;

    case String:
      if (ch == text.at(start)) {
        QChar prev = (i > 0) ? text.at(i - 1) : QChar();
        if (prev != '\\') {
          i++;
          setFormat(start, i - start, colors[ColorComponent::String]);
          nextState = Start;

        } else i++;

      } else i++;
      break;

    case Comment:
      if (ch == '*' && next == '/') {
        i++;
        i++;
        setFormat(start, i - start, colors[ColorComponent::Comment]);
        nextState = Start;

      } else i++;
      break;

    case Regex:
      if (ch == '/') {
        QChar prev = (i > 0) ? text.at(i - 1) : QChar();
        if (prev != '\\') {
          i++;
          setFormat(start, i - start, colors[ColorComponent::String]);
          nextState = Start;

        } else i++;

      } else i++;
      break;

    default:
      nextState = Start;
      break;
    }

    precedingState = state;
    state = nextState;

    if (!ch.isSpace() && state != Comment) lastCh = ch;
  }

  // EOL state
  if (state == Comment)
    setFormat(start, text.length(), colors[ColorComponent::Comment]);
  else state = Start;

  // Record bracket positions
  if (!bracketPositions.isEmpty()) {
    TextBlockData *blockData =
      reinterpret_cast<TextBlockData*>(currentBlock().userData());

    if (!blockData) {
      blockData = new TextBlockData;
      currentBlock().setUserData(blockData);
    }

    blockData->bracketPositions = bracketPositions;
  }

  // Save state
  blockState =
    (state & 0xf) | ((precedingState << 4) & 0xf) | (bracketLevel << 8);
  setCurrentBlockState(blockState);
}
