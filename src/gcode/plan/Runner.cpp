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
#include <cbang/util/SmartFunctor.h>

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
  parsers.push_back(new GCode::Parser(source));
}


bool Runner::hasMore() {return !parsers.empty() || interpreter.hasMore();}


void Runner::next() {
  started = true;

  while (hasMore()) {
    if (!interpreter.hasMore()) {
      interpreter.push(parsers.back());
      parsers.pop_back();
      continue;
    }

    interpreter.next();
    break;
  }
}


bool Runner::execute(const Code &code, int vars) {
  if (parsers.size() < 2 && config.hasOverride(code)) {
    const string &gcode = config.getOverride(code);
    push(StringStreamInputSource(gcode, SSTR("<" << code << ">")));

    return true;
  }

  return false;
}
