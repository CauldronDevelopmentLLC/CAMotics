/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_TOOL_PATH_H
#define OPENSCAM_TOOL_PATH_H

#include "Move.h"

#include <openscam/Geom.h>

#include <vector>
#include <ostream>


namespace cb {namespace JSON {class Sync;}}

namespace OpenSCAM {
  class ToolTable;
  class STL;

  class ToolPath : public std::vector<Move>, public Rectangle3R {
    cb::SmartPointer<ToolTable> tools;

  public:
    ToolPath(const cb::SmartPointer<ToolTable> &tools) : tools(tools) {}
    ~ToolPath();

    const Rectangle3R &getBounds() const {return *this;}
    const cb::SmartPointer<ToolTable> &getTools() const {return tools;}

    void add(const Move &move);
    void print(std::ostream &stream) const;
    void exportJSON(cb::JSON::Sync &sync) const;

    using std::vector<Move>::operator[];
  };
}

#endif // OPENSCAM_TOOL_PATH_H

