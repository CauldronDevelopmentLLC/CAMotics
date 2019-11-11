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

#include "InputCommand.h"

using namespace GCode;
using namespace cb;


InputCommand::InputCommand(port_t port, input_mode_t mode, double timeout) :
  port(port), mode(mode), timeout(timeout) {
  if (timeout) setEntryVelocity(0);
}


void InputCommand::insert(JSON::Sink &sink) const {
  sink.insert("port", String::transcode
              (String::toLower(PortType(port).toString()), "_", "-"));

  switch (mode) {
  case INPUT_RISE: sink.insert("mode", "rise"); break;
  case INPUT_FALL: sink.insert("mode", "fall"); break;
  case INPUT_HIGH: sink.insert("mode", "high"); break;
  case INPUT_LOW:  sink.insert("mode", "low");  break;
  default: sink.insert("mode", "immediate"); break;
  }

  sink.insert("timeout", timeout);
}
