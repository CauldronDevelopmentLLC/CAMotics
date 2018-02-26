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

#include <gcode/Codes.h>

#include <cbang/log/Logger.h>

#include <cbang/io/StringStreamInputSource.h>


using namespace GCode;
using namespace cb;
using namespace std;


Runner::Runner(Controller &controller, const InputSource &source,
               const PlannerConfig &config) :
  config(config), interpreter(controller), started(false) {

  push(source);
  if (!config.programStart.empty())
    push(StringStreamInputSource(config.programStart, "<program-start>"));
}


void Runner::push(const InputSource &source) {
  tokenizers.push_back(new GCode::Tokenizer(source));
}


bool Runner::hasMore() {return !tokenizers.empty();}


void Runner::next() {
  started = true;

  while (!tokenizers.empty()) {
    try {
      if (interpreter.readBlock(*tokenizers.back())) return;
      else tokenizers.pop_back();

    } catch (const EndProgram &) {
      tokenizers.pop_back();

    } catch (const Exception &e) {
      LOG_ERROR(tokenizers.back()->getLocation() << ":" << e.getMessage());
      LOG_DEBUG(3, e);
      tokenizers.clear();
    }
  }
}


bool Runner::execute(const Code &code, int vars) {
  if (tokenizers.size() < 2 && config.hasOverride(code)) {
    const string &gcode = config.getOverride(code);
    push(StringStreamInputSource(gcode, SSTR("<" << code << ">")));

    return true;
  }

  return false;
}
