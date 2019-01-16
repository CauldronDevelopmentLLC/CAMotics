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

#pragma once


#include "Highlighter.h"

#include <gcode/parse/Tokenizer.h>


namespace GCode {class Tokenizer;}

namespace CAMotics {
  class GCodeHighlighter : public Highlighter, public GCode::TokenType  {
    QSet<QString> ops;
    QSet<QString> functions;
    QSet<QString> ocodes;

  public:
    GCodeHighlighter(QTextDocument *parent = 0);

    typedef GCode::Tokenizer::Token_T Token;
    using Highlighter::setFormat;
    void setFormat(const Token &start, const Token &end, ColorComponent cc);
    void setFormat(const Token &token, ColorComponent cc);

  protected:
    // From QSyntaxHighlighter
    void highlightBlock(const QString &text);

    void comment(GCode::Tokenizer &tokenizer);
    void word(GCode::Tokenizer &tokenizer);
    void assign(GCode::Tokenizer &tokenizer);
    void ocode(GCode::Tokenizer &tokenizer);

    void numberRefOrExpr(GCode::Tokenizer &tokenizer);
    void expression(GCode::Tokenizer &tokenizer);
    void unaryOp(GCode::Tokenizer &tokenizer);

    void primary(GCode::Tokenizer &tokenizer);

    void quotedExpr(GCode::Tokenizer &tokenizer);
    void functionCall(GCode::Tokenizer &tokenizer);
    void number(GCode::Tokenizer &tokenizer);
    void reference(GCode::Tokenizer &tokenizer);
  };
}
