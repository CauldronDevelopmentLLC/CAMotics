/******************************************************************************\

             CAMotics is an Open-Source simulation and CAM software.
     Copyright (C) 2011-2021 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "GCodeRunner.h"

using namespace GCode;
using namespace cb;


GCodeRunner::GCodeRunner(Controller &controller, const InputSource &source,
                         const PlannerConfig &config) :
  Runner(config), interp(controller) {

  for (auto it = config.overrides.begin(); it != config.overrides.end(); it++)
    interp.addOverride(it->first, it->second);

  interp.push(source);
  if (!config.programStart.empty())
    interp.push(config.programStart, "<program-start>");
}


void GCodeRunner::step() {
  try {
    interp(interp.next());

  } catch (const EndProgram &) {
    interp.unwind();
  }
}
