/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#ifndef CAMOTICS_TOKENIZER_H
#define CAMOTICS_TOKENIZER_H

#include "Token.h"

#include <cbang/parse/Tokenizer.h>


namespace CAMotics {
  class Tokenizer : public cb::Tokenizer<TokenType> {
  public:
    using cb::Tokenizer<TokenType>::Token_T;

    Tokenizer(cb::Scanner &scanner) : cb::Tokenizer<TokenType>(scanner)
    {advance();}

    bool isID(const std::string &id) const;

  protected:
    void comment();
    void parenComment();
    void number(bool positive = true);
    void id();

    // From cb::Tokenizer
    void next();
  };
}

#endif // CAMOTICS_TOKENIZER_H

