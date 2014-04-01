/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_LINE_BUFFER_H
#define OPENSCAM_LINE_BUFFER_H

#include <cbang/os/Mutex.h>
#include <cbang/util/SmartLock.h>

#include <string>
#include <list>


namespace OpenSCAM {
  class LineBuffer : public cb::Mutex {
    std::list<std::string> lines;

  public:
    void append(const std::string &line) {
      cb::SmartLock lock(this);
      lines.push_back(line);
    }

    bool hasLine() const {
      cb::SmartLock lock(this);
      return !lines.empty();
    }

    std::string getLine() {
      cb::SmartLock lock(this);
      if (lines.empty()) return "";

      std::string line = lines.front();
      lines.pop_front();

      return line;
    }
  };
}

#endif // OPENSCAM_LINE_BUFFER_H

