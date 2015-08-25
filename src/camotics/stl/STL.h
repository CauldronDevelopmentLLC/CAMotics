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

#include <cbang/Packed.h>
#include <cbang/io/IO.h>

#include <iostream>
#include <vector>

namespace CAMotics {
  class Task;

  class STL : public cb::IO, public std::vector<Facet> {
    std::string name;
    std::string hash;
    bool binary;

    struct BinaryTriangle {
      float normal[3];
      float v1[3];
      float v2[3];
      float v3[3];
      uint16_t attrib;
    } PACKED;

  public:
    STL(const std::string &name = std::string(),
        const std::string &hash = std::string()) :
      name(name), hash(hash), binary(false) {}

    const std::string &getName() const {return name;}
    void setName(const std::string &name) {this->name = name;}

    const std::string &getHash() const {return hash;}
    void setHash(const std::string &hash) {this->hash = hash;}

    bool isBinary() const {return binary;}
    void setBinary(bool binary) {this->binary = binary;}

    void add(const Facet &f) {push_back(f);}
    void addFacet(const cb::Vector3F &v0, const cb::Vector3F &v1,
                  const cb::Vector3F &v2, const cb::Vector3F &normal)
    {add(Facet(v0, v1, v2, normal));}

    void readHeader(const cb::InputSource &source);
    void writeHeader(const cb::OutputSink &sink) const;
    void readBody(const cb::InputSource &source, Task *task = 0);
    void writeBody(const cb::OutputSink &sink, Task *task = 0) const;
    void read(const cb::InputSource &source, Task *task);
    void write(const cb::OutputSink &sink, Task *task) const;

    // From IO
    void read(const cb::InputSource &source) {read(source, 0);}
    void write(const cb::OutputSink &sink) const {write(sink, 0);}
  };
}

#endif // CAMOTICS_STL_H
