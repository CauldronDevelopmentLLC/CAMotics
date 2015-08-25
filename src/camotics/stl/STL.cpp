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

#include "STL.h"

#include <camotics/Task.h>

#include <cbang/Exception.h>
#include <cbang/String.h>
#include <cbang/io/Parser.h>

#include <string.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


void STL::readHeader(const InputSource &source) {
  istream &stream = source.getStream();

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
  }

  if (stream.bad()) THROWS("Error while reading STL header");
}


void STL::writeHeader(const OutputSink &sink) const {
  ostream &stream = sink.getStream();

  if (binary) {
    // Header
    char header[80];
    memset(header, 0, 80);
    if (!hash.empty()) strncpy(header, hash.c_str(), 80);
    stream.write(header, 80);

    // Count
    uint32_t count = size();
    stream.write((char *)&count, 4);

  } else {
    stream << scientific << "solid " << name;
    if (!hash.empty()) stream << " " << hash;
    stream << '\n';
  }

  if (stream.bad()) THROWS("Error while writing STL header");
}


void STL::readBody(const InputSource &source, Task *task) {
  istream &stream = source.getStream();

  if (binary) {
    // Count
    uint32_t count = 0;
    stream.read((char *)&count, 4);
    reserve(count);

    // Triangles
    BinaryTriangle tri;
    for (unsigned i = 0; i < count && !stream.fail(); i++) {
      stream.read((char *)&tri, sizeof(tri));

      push_back(Facet(Vector3F(tri.v1), Vector3F(tri.v2), Vector3F(tri.v3),
                      Vector3F(tri.normal)));

      if (task) task->update((double)i / count, "Reading STL");
    }

  } else {
    Parser parser(stream);
    parser.setCaseSensitive(false);
    parser.setLine(2);
    parser.setCol(0);

    double progress = 0;

    try {
      Vector3F normal;
      Vector3F vertices[3];

      while (!stream.fail() && parser.check("facet")) {
        parser.advance();
        parser.match("normal");

        for (unsigned i = 0; i < 3; i++)
          normal[i] = String::parseDouble(parser.advance());

        parser.match("outer");
        parser.match("loop");

        for (unsigned i = 0; i < 3; i++) {
          parser.match("vertex");

          for (unsigned j = 0; j < 3; j++)
            vertices[i][j] = String::parseDouble(parser.advance());
        }

        parser.match("endloop");
        parser.match("endfacet");

        // Add it
        push_back(Facet(vertices[0], vertices[1], vertices[2], normal));

        if (task) {
          progress += 0.001;
          if (1 < progress) progress = 0;
          task->update(progress, "Reading STL");
        }
      }

      parser.match("endsolid");
      char buffer[1024];
      stream.getline(buffer, 1023); // Name

    } catch (const Exception &e) {
      throw Exception("Parse error", parser, e);
    }
  }

  if (task) task->update(1, "Reading STL");
  if (stream.bad()) THROWS("Error while reading STL body");
}


static void writePoint(ostream &stream, const Vector3F &pt) {
  stream << pt.x() << ' ' << pt.y() << ' ' << pt.z();
}


void STL::writeBody(const OutputSink &sink, Task *task) const {
  ostream &stream = sink.getStream();

  if (binary) {
    BinaryTriangle tri;
    tri.attrib = 0;

    for (unsigned i = 0; i < size(); i++) {
      const Facet &facet = (*this)[i];

      for (unsigned j = 0; j < 3; j++) {
        tri.normal[j] = (float)facet.getNormal()[j];

        tri.v1[j] = (float)facet[0][j];
        tri.v2[j] = (float)facet[1][j];
        tri.v3[j] = (float)facet[2][j];
      }

      stream.write((char *)&tri, sizeof(tri));

      if (task) task->update((double)i / size(), "Writing STL");
    }

  } else {
    for (unsigned i = 0; i < size(); i++) {
      const Facet &facet = (*this)[i];

      stream << "facet normal ";
      writePoint(stream, facet.getNormal());
      stream << "\nouter loop";
      for (unsigned j = 0; j < 3; j++) {
        stream << "\nvertex ";
        writePoint(stream, facet[j]);
      }
      stream << "\nendloop\nendfacet\n";

      if (task) task->update((double)i / size(), "Writing STL");
    }

    stream << "endsolid " << name;
    if (!hash.empty()) stream << " " << hash;
    stream << '\n';
  }

  if (task) task->update(1, "Writing STL");
  if (stream.bad()) THROWS("Error while writing STL body");
}


void STL::read(const InputSource &source, Task *task) {
  readHeader(source);
  readBody(source, task);
}


void STL::write(const OutputSink &sink, Task *task) const {
  writeHeader(sink);
  writeBody(sink, task);
}
