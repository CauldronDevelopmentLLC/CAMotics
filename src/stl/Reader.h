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


#include "Source.h"

#include <cbang/io/InputSource.h>
#include <cbang/io/Parser.h>


namespace STL {
  class Reader : public Source {
    cb::InputSource source;
    std::istream &stream;
    bool binary;
    uint32_t count;
    cb::Parser parser;

  public:
    Reader(const cb::InputSource &source);

    uint32_t readHeader(std::string &name, std::string &hash);
    uint32_t getFacetCount() const {return count;}
    bool hasMore();
    void readFacet(cb::Vector3F &v1, cb::Vector3F &v2, cb::Vector3F &v3,
                   cb::Vector3F &normal);
    void readFooter();
  };
}
