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

#include "SetCommand.h"

using namespace cb;
using namespace GCode;


void SetCommand::insert(JSON::Sink &sink) const {
  sink.insert("name", name);
  sink.insert("value", *value);
}


void SetCommand::write(MachineInterface &machine) const {
  if (name == "_feed") machine.setFeed(value->getNumber());
  else if (name == "_tool") machine.changeTool(value->getU32());
  else if (name == "speed") machine.setSpeed(value->getNumber());
  else if (name == "message") machine.message(value->getString());
  else if (name == "spin-mode") ; // TODO machine.setSpinMode()
  else if (name == "max-rpm")   ; // TODO machine.setSpinMode()
  else if (name == "line")      ; // TODO machine.setLocation()
  else if (value->isNumber())
    machine.set(name, value->getNumber(), Units::METRIC);
}
