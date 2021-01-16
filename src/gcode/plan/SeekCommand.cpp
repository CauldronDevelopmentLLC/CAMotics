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

#include "SeekCommand.h"

using namespace GCode;
using namespace cb;


void SeekCommand::insert(JSON::Sink &sink) const {
  sink.insert("switch", String::transcode
              (String::toLower(PortType(port).toString()), "_", "-"));
  sink.insertBoolean("active", active);
  sink.insertBoolean("error", error);
}


void SeekCommand::write(MachineInterface &machine) const {
  machine.seek(port, active, error);
}
