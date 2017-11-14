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

#include <camotics/Application.h>
#include <camotics/contour/Triangle.h>

#include <stl/Writer.h>

#include <cbang/Exception.h>
#include <cbang/ApplicationMain.h>
#include <cbang/io/StringInputSource.h>
#include <cbang/json/Reader.h>
#include <cbang/util/DefaultCatch.h>
#include <cbang/geom/Vector.h>

#include <vector>
#include <iostream>

using namespace std;
using namespace cb;


class TC02STLApp : public CAMotics::Application {
  SmartPointer<STL::Writer> writer;
  vector<Triangle3F> triangles;

public:
  TC02STLApp() : CAMotics::Application("TCO to STL converter") {}


  void parseBlock(const cb::InputSource &source) {
    vector<Vector2U> lines;
    vector<Vector3U> triangles;
    vector<Vector3F> vertices;

    int numTriangles = -1;
    int numLines = -1;
    int numVertices = -1;

    while (source.getStream().good()) {
      string line = String::trim(source.getLine());
      if (line.empty()) break;

      switch (line[0]) {
      case 't':
      case 'l':
      case 'v': {
        size_t equal = line.find('=');
        if (equal == string::npos) continue;

        int start = 1;
        while (line[start] == '0') start++;
        unsigned n = String::parseU32(line.substr(start));

        vector<string> nums;
        String::tokenize(line.substr(equal + 1), nums, ",");

        if (line[0] == 't' && nums.size() == 3)
          triangles.push_back(Vector3U(String::parseU32(nums[0]),
                                       String::parseU32(nums[1]),
                                       String::parseU32(nums[2])));

        else if (line[0] == 'l' && nums.size() == 2)
          lines.push_back(Vector2U(String::parseU32(nums[0]),
                                   String::parseU32(nums[1])));

        else if (line[0] == 'v' && nums.size() == 3) {
          if (vertices.size() < n + 1) vertices.resize((n + 1) * 1.5);
          vertices[n] = Vector3D(String::parseFloat(nums[0]),
                                 String::parseFloat(nums[1]),
                                 String::parseFloat(nums[2]));
        }

        break;
      }

      case 'n':
        if (1 < line.size())
          switch (line[1]) {
          case 't': numTriangles = String::parseU32(line.substr(3)); break;
          case 'l': numLines = String::parseU32(line.substr(3)); break;
          case 'v': numVertices = String::parseU32(line.substr(3)); break;
          }
        break;

      default: LOG_DEBUG(1, line); break;
      }
    }

    //if (0 <= numLines) lines.resize(numLines);
    //if (0 <= numTriangles) triangles.resize(numTriangles);
    //if (0 <= numVertices) vertices.resize(numVertices);

    LOG_INFO(1, "lines=" << numLines << " triangles=" << numTriangles
             << " vertices=" << numVertices);

    // Assemble triangles
    for (unsigned i = 0; i < triangles.size(); i++) {
      const Vector3U &indices = triangles[i];
      this->triangles.push_back(Triangle3F(vertices[indices[0]],
                                           vertices[indices[1]],
                                           vertices[indices[2]]));
    }
  }


  // From cb::Reader
  void read(const cb::InputSource &source) {
    while (source.getStream().good()) {
      string line = String::trim(source.getLine());
      if (!line.empty() && line[0] == '[') parseBlock(source);
    }

    STL::Writer writer(cout, true);

    writer.writeHeader("tco2stl", triangles.size());

    for (unsigned i = 0; i < triangles.size(); i++) {
      const Triangle3F &t = triangles[i];
      writer.writeFacet(t, CAMotics::Triangle::computeNormal(t));
    }

    writer.writeFooter("tco2stl");
  }
};


int main(int argc, char *argv[]) {
  return cb::doApplication<TC02STLApp>(argc, argv);
}
