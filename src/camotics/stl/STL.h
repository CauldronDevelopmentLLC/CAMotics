/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#ifndef CAMOTICS_STL_H
#define CAMOTICS_STL_H

#include "Facet.h"

#include <cbang/io/IO.h>

#include <iostream>
#include <vector>

namespace CAMotics {
  class Task;
  class STLSink;
  class STLSource;

  class STL : public std::vector<Facet> {
    std::string name;
    std::string hash;

  public:
    STL(const std::string &name = std::string(),
        const std::string &hash = std::string()) :
      name(name), hash(hash) {}

    const std::string &getName() const {return name;}
    void setName(const std::string &name) {this->name = name;}

    const std::string &getHash() const {return hash;}
    void setHash(const std::string &hash) {this->hash = hash;}

    void add(const Facet &f) {push_back(f);}

    void read(STLSource &source, Task *task = 0);
    void write(STLSink &sink, Task *task = 0) const;

    void read(const cb::InputSource &source, Task *task = 0);
    void write(const cb::OutputSink &sink, Task *task = 0,
               bool binary = true) const;
  };
}

#endif // CAMOTICS_STL_H
