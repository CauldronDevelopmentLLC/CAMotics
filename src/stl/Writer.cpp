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

#include "Writer.h"
#include "BinaryTriangle.h"

#include <string.h>

using namespace std;
using namespace cb;
using namespace STL;


void Writer::writeHeader(const string &name, uint32_t count,
                         const string &hash) {
  if (binary) {
    // Header
    char header[81];
    memset(header, 0, 81);
    if (!hash.empty()) memcpy(header, hash.c_str(), 80);
    stream.write(header, 80);

    // Count
    stream.write((char *)&count, 4);

  } else {
    stream << scientific << "solid " << name;
    if (!hash.empty()) stream << " " << hash;
    stream << '\n';
  }
}


static void writePoint(ostream &stream, const Vector3F &pt) {
  stream << pt.x() << ' ' << pt.y() << ' ' << pt.z();
}


void Writer::writeFacet(const Vector3F &v1, const Vector3F &v2,
                        const Vector3F &v3, const Vector3F &normal) {
  if (binary) {
    BinaryTriangle tri;
    tri.attrib = 0;

    for (unsigned i = 0; i < 3; i++) {
      tri.normal[i] = normal[i];
      tri.v1[i] = v1[i];
      tri.v2[i] = v2[i];
      tri.v3[i] = v3[i];
    }

    stream.write((char *)&tri, 50);

  } else {
    stream << "facet normal ";
    writePoint(stream, normal);

    stream << "\nouter loop";

    stream << "\nvertex ";
    writePoint(stream, v1);
    stream << "\nvertex ";
    writePoint(stream, v2);
    stream << "\nvertex ";
    writePoint(stream, v3);

    stream << "\nendloop\nendfacet\n";
  }
}


void Writer::writeFacet(const Triangle3F &t, const Vector3F &normal) {
  writeFacet(t[0], t[1], t[2], normal);
}


void Writer::writeFooter(const string &name, const string &hash) {
  if (!binary) {
    stream << "endsolid " << name;
    if (!hash.empty()) stream << " " << hash;
    stream << '\n';
  }
}
