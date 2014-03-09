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

#include "STL.h"

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/io/Parser.h>

#include <string.h>

#include <cctype>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void STL::read(const cb::InputSource &source) {
  istream &stream = source.getStream();

  char buffer[1024];
  stream.read(buffer, 6);
  streamsize count = stream.gcount();

  if (6 == count && String::toLower(string(buffer, 5)) == "solid" &&
      isspace(buffer[5])) { // ASCII
    // Name
    stream.getline(buffer, 1023);
    buffer[1023] = 0; // Terminate
    name = String::trim(buffer);

    // Parse
    Parser parser(stream);
    parser.setCaseSensitive(false);
    parser.setLine(1);
    parser.setCol(0);

    try {
      while (!stream.fail()) {
        if (!parser.check("facet")) break;
        parser.match("normal");

        Vector3F normal(String::parseDouble(parser.advance()),
                        String::parseDouble(parser.advance()),
                        String::parseDouble(parser.advance()));

        parser.match("outer");
        parser.match("loop");

        Vector3F pts[3];
        for (int i = 0; i < 3; i++) {
          parser.match("vertex");
          pts[i] = Vector3F(String::parseDouble(parser.advance()),
                            String::parseDouble(parser.advance()),
                            String::parseDouble(parser.advance()));
        }

        parser.match("endloop");
        parser.match("endfacet");

        // Add it
        push_back(Facet(pts[0], pts[1], pts[2], normal));
      }

      parser.match("endsolid");
      stream.getline(buffer, 1023); // Name

    } catch (const Exception &e) {
      throw Exception("Parse error", parser, e);
    }

  } else { // Binary
    // Skip header
    stream.seekg(80, ios::beg);

    // Read count
    uint32_t count = 0;
    stream.read((char *)&count, 4);

    BinaryTriangle tri;
    while (count && !stream.fail()) {
      stream.read((char *)&tri, sizeof(tri));

      push_back(Facet(Vector3F(tri.v1), Vector3F(tri.v2), Vector3F(tri.v3),
                      Vector3F(tri.normal)));

      count--;
    }
  }

  if (stream.bad()) THROWS("Error while parsing STL");
}


static void writePoint(ostream &stream, const Vector3F &pt) {
  stream << pt.x() << ' ' << pt.y() << ' ' << pt.z();
}


void STL::write(const cb::OutputSink &sync) const {
  ostream &stream = sync.getStream();

  if (binary) {
    // Header
    char header[80];
    memset(header, 0, 80);
    stream.write(header, 80);

    // Count
    uint32_t count = size();
    stream.write((char *)&count, 4);

    // Triangles
    BinaryTriangle tri;
    tri.attrib = 0;
    for (const_iterator it = begin(); it != end(); it++) {
      const Facet &facet = *it;

      for (unsigned i = 0; i < 3; i++) {
        tri.normal[i] = (float)facet.getNormal()[i];
        tri.v1[i] = (float)facet[0][i];
        tri.v2[i] = (float)facet[1][i];
        tri.v3[i] = (float)facet[2][i];
      }

      stream.write((char *)&tri, sizeof(tri));
    }

  } else {
    stream << scientific << "solid " << name;

    for (const_iterator it = begin(); it != end(); it++) {
      const Facet &facet = *it;

      stream << "\nfacet normal ";
      writePoint(stream, facet.getNormal());
      stream << "\nouter loop";
      stream << "\nvertex "; writePoint(stream, facet[0]);
      stream << "\nvertex "; writePoint(stream, facet[1]);
      stream << "\nvertex "; writePoint(stream, facet[2]);
      stream << "\nendloop\nendfacet";
    }

    stream << "\nendsolid " << name << '\n';
  }
}
