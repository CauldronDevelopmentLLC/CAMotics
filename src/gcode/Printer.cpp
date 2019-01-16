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

#include "Printer.h"

#include "Codes.h"

#include "ast/Word.h"
#include "ast/Number.h"

#include <cbang/SStream.h>
#include <cbang/Math.h>
#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace GCode;


void Printer::operator()(const SmartPointer<Block> &block) {
  if (!block->isEmpty() || !removeBlankLines) {
    if (annotate) {
      string comments;

      // Find Words
      Block::const_iterator it;
      for (it = block->begin(); it != block->end(); it++)
        if (it->isInstance<Word>()) {
          SmartPointer<Word> word = it->cast<Word>();
          char type = word->getType();
          const SmartPointer<Entity> &expr = word->getExpression();

          if (expr.isInstance<Number>()) {
            double value = expr.cast<Number>()->getValue();
            const Code *code = Codes::find(type, value);
            if (code) comments += SSTR(" (" << code->description << ')');
          }
        }

      // Try to format to 80 columns
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
