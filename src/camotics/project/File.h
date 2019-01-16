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


#include <cbang/StdTypes.h>

#include <string>


namespace CAMotics {
  namespace Project {
    class File {
      std::string path;
      uint64_t modTime;

    public:
      File(const std::string &path);

      bool isTPL() const;
      bool isDXF() const;

      void setPath(const std::string &path);
      const std::string &getPath() const {return path;}
      std::string getRelativePath(const std::string &dir) const;
      std::string getBasename() const;

      bool exists() const;
      uint64_t getTime() const;
      bool changed() const {return modTime < getTime();}
      void update() {modTime = getTime();}
    };
  }
}
