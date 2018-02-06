/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "Runner.h"

#include <cbang/log/Logger.h>


using namespace GCode;
using namespace cb;
using namespace std;


Runner::Runner(Controller &controller, const InputSource &source) :
  interpreter(controller), scanner(source), tokenizer(scanner), started(false) {
}


bool Runner::next() {
  started = true;

  try {
    return interpreter.readBlock(tokenizer);

  } catch (const EndProgram &) {
  } catch (const Exception &e) {
    LOG_ERROR(tokenizer.getLocation() << ":" << e.getMessage());
    LOG_DEBUG(3, e);
  }

  return false;
}
