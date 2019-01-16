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

#include "File.h"

#include <cbang/json/Serializable.h>

#include <vector>


namespace CAMotics {
  namespace Project {
    class Files : public cb::JSON::Serializable {
      std::string directory;
      std::vector<cb::SmartPointer<File> > files;

    public:
      Files(const std::string &directory) : directory(directory) {}

      const std::string &getDirectory() const {return directory;}
      void setDirectory(const std::string &dir) {directory = dir;}

      unsigned size() const {return files.size();}

      const cb::SmartPointer<File> &get(unsigned index) const;
      std::string getRelativePath(unsigned index) const;
      cb::SmartPointer<File> find(const std::string &path) const;
      void add(const std::string &filename);
      void remove(unsigned i) {files.erase(files.begin() + i);}

      // From JSON::Serializable
      void read(const cb::JSON::Value &value);
      void write(cb::JSON::Sink &sink) const;

      static std::string encode(const std::string &filename);
      static std::string decode(const std::string &filename);
    };
  }
}
