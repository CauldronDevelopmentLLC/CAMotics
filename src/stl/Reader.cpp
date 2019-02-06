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

#include "Reader.h"
#include "BinaryTriangle.h"

#include <cbang/String.h>

using namespace std;
using namespace cb;
using namespace STL;


Reader::Reader(const InputSource &source) :
  source(source), stream(source.getStream()), binary(true), count(0),
  parser(source.getStream()) {
  parser.setCaseSensitive(false);
}


uint32_t Reader::readHeader(string &name, string &hash) {
  char buffer[1024];
  stream.read(buffer, 6);

  if (stream.gcount() == 6 && String::toLower(string(buffer, 6)) == "solid ") {
    // ASCII
    binary = false;

    // Name
    stream.getline(buffer, 1023);
    buffer[1023] = 0; // Terminate
    name = String::trim(buffer);

    // Hash
    string::size_type pos = name.find_last_of(' ');
    if (pos != string::npos) {
      hash = name.substr(pos + 1);
      name = String::trim(name.substr(0, pos));
    }

  } else {
    // Binary
    binary = true;

    // Read rest of header
    stream.read(buffer + 6, 74);
    buffer[80] = 0; // Make sure we have null terminator
    hash = string(buffer);

    // Count
    stream.read((char *)&count, 4);
  }

  return count;
}


bool Reader::hasMore() {
  return !stream.fail() &&
    ((binary && count) || (!binary && parser.check("facet")));
}


void Reader::readFacet(Vector3F &v1, Vector3F &v2, Vector3F &v3,
                       Vector3F &normal) {
  if (binary) {
    BinaryTriangle tri;
    stream.read((char *)&tri, 50);

    v1 = tri.v1;
    v2 = tri.v2;
    v3 = tri.v3;
    normal = tri.normal;

    count--;

  } else {
    parser.advance();
    parser.match("normal");

    for (unsigned i = 0; i < 3; i++)
      normal[i] = String::parseDouble(parser.advance());

    parser.match("outer");
    parser.match("loop");

    Vector3F *vertices[3] = {&v1, &v2, &v3};

    for (unsigned i = 0; i < 3; i++) {
      parser.match("vertex");

      for (unsigned j = 0; j < 3; j++)
        (*vertices[i])[j] = String::parseDouble(parser.advance());
    }

    parser.match("endloop");
    parser.match("endfacet");
  }
}


void Reader::readFooter() {
  if (!binary) {
    parser.match("endsolid");
    char buffer[1024];
    stream.getline(buffer, 1023); // Name
  }
}
