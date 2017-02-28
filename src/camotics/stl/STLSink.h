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

#ifndef CAMOTICS_STLSINK_H
#define CAMOTICS_STLSINK_H

#include <cbang/StdTypes.h>
#include <cbang/geom/Vector.h>

#include <string>


namespace CAMotics {
  class Facet;

  class STLSink {
  public:
    virtual ~STLSink() {}

    virtual void writeHeader(const std::string &name, uint32_t count,
                             const std::string &hash = std::string()) = 0;
    virtual void writeFacet(const cb::Vector3F &v1, const cb::Vector3F &v2,
                            const cb::Vector3F &v3,
                            const cb::Vector3F &normal) = 0;
    virtual void writeFooter(const std::string &name,
                             const std::string &hash = std::string()) = 0;

    void writeFacet(const Facet &facet);
  };
}

#endif // CAMOTICS_STLSINK_H
