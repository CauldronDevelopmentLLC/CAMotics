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

#include "Block.h"

#include "Word.h"
#include "OCode.h"

#include <iostream>

using namespace std;
using namespace GCode;


Word *Block::findWord(char type, double number) const {
  Word *word;

  for (const_iterator it = begin(); it != end(); it++)
    if ((word = (*it)->instance<Word>()) && word->getType() == type &&
        word->getValue() == number) return word;

  return 0;
}


Word *Block::findWord(char type) const {
  Word *word;

  for (const_iterator it = begin(); it != end(); it++)
    if ((word = (*it)->instance<Word>()) && word->getType() == type)
      return word;

  return 0;
}


OCode *Block::findOCode() const {
  OCode *ocode = 0;
  for (const_iterator it = begin(); it != end(); it++)
    if ((ocode = (*it)->instance<OCode>())) break;
  return ocode;
}


void Block::print(ostream &stream) const {
  if (deleted) stream << '/';
  if (line != -1) stream << 'N' << line << ' ';

  for (const_iterator it = begin(); it != end(); it++) {
    if (it != begin()) stream << ' ';
    stream << **it;
  }
}
