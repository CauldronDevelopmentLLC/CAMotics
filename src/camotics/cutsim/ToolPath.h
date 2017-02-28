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

#pragma once


#include "MoveStream.h"

#include <camotics/Geom.h>
#include <camotics/sim/ToolTable.h>

#include <cbang/json/Serializable.h>

#include <vector>
#include <ostream>


namespace cb {namespace JSON {class Sink;}}

namespace CAMotics {
  class STL;

  class ToolPath :
    public std::vector<Move>, public Rectangle3R, public MoveStream,
    public cb::JSON::Serializable {
    ToolTable tools;

  public:
    ToolPath(const ToolTable &tools) : tools(tools) {}
    ~ToolPath();

    const Rectangle3R &getBounds() const {return *this;}
    const ToolTable &getTools() const {return tools;}
    ToolTable &getTools() {return tools;}

    int find(real time, unsigned first, unsigned last) const;
    int find(real time) const;

    void print() const {}

    // From cb::JSON::Serializable
    using cb::JSON::Serializable::read;
    using cb::JSON::Serializable::write;
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;

    using std::vector<Move>::operator[];

    // From MoveStream
    void move(Move &move);
  };
}
