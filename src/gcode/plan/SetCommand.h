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

#pragma once

#include "PlannerCommand.h"

#include <cbang/json/Value.h>


namespace GCode {
  class SetCommand : public PlannerCommand {
    const std::string name;
    cb::SmartPointer<cb::JSON::Value> value;

  public:
    SetCommand(const std::string &name,
               const cb::SmartPointer<cb::JSON::Value> &value) :
      name(name), value(value) {}

    const std::string &getName() const {return name;}
    const cb::JSON::Value &getValue() const {return *value;}
    void setValue(const cb::SmartPointer<cb::JSON::Value> &value)
    {this->value = value;}

    // From PlannerCommand
    const char *getType() const {return "set";}
    void insert(cb::JSON::Sink &sink) const;
    void write(MachineInterface &machine) const;
  };
}
