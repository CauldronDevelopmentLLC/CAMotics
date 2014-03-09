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

#ifndef OPENSCAM_STL_H
#define OPENSCAM_STL_H

#include "Facet.h"

#include <cbang/Packed.h>
#include <cbang/io/IO.h>

#include <iostream>
#include <vector>

namespace OpenSCAM {
  class STL : public cb::IO, public std::vector<Facet> {
    std::string name;
    bool binary;

    struct BinaryTriangle {
      float normal[3];
      float v1[3];
      float v2[3];
      float v3[3];
      uint16_t attrib;
    } PACKED;

  public:
    STL(const std::string &name = std::string()) : name(name), binary(false) {}

    const std::string &getName() const {return name;}
    void setName(const std::string &name) {this->name = name;}

    bool isBinary() const {return binary;}
    void setBinary(bool binary) {this->binary = binary;}

    void add(const Facet &f) {push_back(f);}
    void addFacet(const cb::Vector3F &v0, const cb::Vector3F &v1,
                  const cb::Vector3F &v2, const cb::Vector3F &normal)
    {add(Facet(v0, v1, v2, normal));}

    // From IO
    void read(const cb::InputSource &source);
    void write(const cb::OutputSink &sync) const;
  };
}

#endif // OPENSCAM_STL_H
