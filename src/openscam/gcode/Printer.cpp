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

#include "Printer.h"

#include <cbang/SStream.h>
#include <cbang/log/Logger.h>

#include "Codes.h"

#include "ast/Word.h"
#include "ast/Number.h"

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void Printer::operator()(const SmartPointer<Block> &block) {
  if (!block->isEmpty() || !removeBlankLines) {
    if (addComments) {
      string comments;

      Block::const_iterator it;
      for (it = block->begin(); it != block->end(); it++) {
        Word *word = dynamic_cast<Word *>(it->get());

        if (word) {
          char type = word->getType();
          Number *number = dynamic_cast<Number *>(word->getExpression().get());

          if (number) {
            double value = number->getValue();
            const Code *code = Codes::find(type, value);
            
            if (code) comments += SSTR(" (" << code->description << ')');
          }
        }
      }

      if (!comments.empty()) {
        string line = SSTR(*block);
        stream << line;
        
        int pad = 80 - (line.length() + comments.length());
        if (0 < pad) stream << string(pad, ' ');
        
        stream << comments << '\n';

        return;
      }
    }

    stream << *block << '\n';
  }
}
