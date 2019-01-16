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

#include "STLModule.h"
#include "TPLContext.h"

#include <stl/Reader.h>
#include <stl/Facet.h>

#include <cbang/io/InputSource.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/geom/Segment.h>
#include <cbang/geom/Rectangle.h>
#include <cbang/log/Logger.h>

#include <limits>
#include <list>
#include <vector>
#include <cmath>

using namespace tplang;
using namespace cb;
using namespace std;


namespace {
  bool isBetween(float level, float z1, float z2) {
    return (z1 <= level && level <= z2) || (z2 <= level && level <= z1);
  }


  float distanceBetweenPoints(const Vector2F &p1, const Vector2F &p2) {
    float distance = p1.distance(p2);
    return distance < 0.001 ? 0 : distance;
  }


  bool findSegment(float level, const STL::Facet &f, Vector2F &p1,
                   Vector2F &p2) {
    // skip facets that lay on the "level" plane
    if (f[0].z() == level && f[1].z() == level && f[2].z() == level)
      return false;

    // skip faces with an edge on the "level" plane and hang down
    if ((f[0].z() == level && f[1].z() == level && f[2].z() < level) ||
        (f[0].z() == level && f[2].z() == level && f[1].z() < level) ||
        (f[1].z() == level && f[2].z() == level && f[0].z() < level))
      return false;

    bool p1Taken = false;
    bool p2Taken = false;

    if (isBetween(level, f[0].z(), f[1].z())) {
      if (f[0].z() == f[1].z()) { // edge is on the plane
        if (f[2].y() > level) return false;

        p1 = f[0].slice<2>();
        p2 = f[1].slice<2>();

        return true;
      }

      float ratio = (level - f[1].z()) / (f[0].z() - f[1].z());
      p1 = (f[1] + (f[0] - f[1]) * ratio).slice<2>();
      p1Taken = true;
    }

    if (isBetween(level, f[1].z(), f[2].z())) {
      if (f[1].z() == f[2].z()) { // edge is on the plane
        if (f[0].y() > level) return false;

        p1 = f[1].slice<2>();
        p2 = f[2].slice<2>();

        return true;
      }

      float ratio = (level - f[2].z()) / (f[1].z() - f[2].z());

      if (!p1Taken) {
        p1 = (f[2] + (f[1] - f[2]) * ratio).slice<2>();
        p1Taken = true;

      } else {
        p2 = (f[2] + (f[1] - f[2]) * ratio).slice<2>();
        p2Taken = true;
      }
    }

    if (!p1Taken) return false;

    if (isBetween(level, f[2].z(), f[0].z())) {
      float ratio = (level - f[0].z()) / (f[2].z() - f[0].z());

      if (!p2Taken) {
        p2 = (f[0] + (f[2] - f[0]) * ratio).slice<2>();
        p2Taken = true;
      }
    }

    if (!p2Taken) return false;
    return distanceBetweenPoints(p1, p2); // don't return single point segs
  }


  void swap(Vector2F &p1, Vector2F &p2) {
    Vector2F temp = p1;
    p1 = p2;
    p2 = temp;
  }


  bool isEqual(float x, float y) {
    if (y) return fabs((x - y) / y) < 0.00001;
    return fabs(x) < 0.00001;
  }


  void append(js::Sink &sink, const Vector2F &v) {
    sink.appendDict();
    sink.insert("X", v.x());
    sink.insert("Y", v.y());
    sink.endDict();
  }


  void append(js::Sink &sink, const Vector3F &v) {
    sink.appendList();
    for (int i = 0; i < 3; i++) sink.append(v[i]);
    sink.endList();
  }


  Vector3F toVector3F(const js::Value &value) {
    return Vector3F(value.getNumber(0), value.getNumber(1), value.getNumber(2));
  }


  void readFacets(vector<STL::Facet> &out, const js::Value &in) {
    unsigned length = in.length();
    STL::Facet f;

    for (unsigned i = 0; i < length; i++) {
      SmartPointer<js::Value> facet = in.get(i);

      for (unsigned j = 0; j < 3; j++)
        f[j] = toVector3F(*facet->get(j));

      f.getNormal() = toVector3F(*facet->get(3));

      out.push_back(f);
    }
  }


  void contourLevel(js::Sink &sink, const vector<STL::Facet> &facets,
                    float level) {
    // Find segments
    typedef list<Segment2F> segments_t;
    segments_t segments;

    for (unsigned i = 0; i < facets.size(); i++) {
      const STL::Facet &f = facets[i];
      Vector2F p1, p2;

      if ((level < f[0].z() && level < f[1].z() && level < f[2].z()) ||
          (f[0].z() < level && f[1].z() < level && f[2].z() < level))
        continue;

      if (findSegment(level, f, p1, p2)) {
        if (isEqual(p1.x(), p2.x())) {
          if (p1.y() < p2.y()) {
            if (f.getNormal().x() < 0) swap(p1, p2);
          } else if (0 < f.getNormal().x()) swap(p1, p2);

        } else if (isEqual(p1.y(), p2.y())) {
          if (p1.x() < p2.x()) {
            if (0 < f.getNormal().y()) swap(p1, p2);
          } else if (f.getNormal().y() < 0) swap(p1, p2);

        } else if (p1.x() < p2.x() && p1.y() < p2.y()) {
          if (f.getNormal().x() < 0) swap(p1, p2);

        } else if (p2.x() < p1.x() && p2.y() < p1.y()) {
          if (0 < f.getNormal().x()) swap(p1, p2);

        } else if (p1.x() < p2.x() && p2.y() < p1.y()) {
          if (0 < f.getNormal().x()) swap(p1, p2);

        } else if (f.getNormal().x() < 0) swap(p1, p2);

        Segment2F seg(p1, p2);
        segments.push_back(seg);
      }
    }

    // Link segments
    bool inLoop = false;
    Vector2F first;
    Vector2F last;

    sink.beginList();

    while (!segments.empty()) {
      // Start new loop
      if (!inLoop) {
        inLoop = true;
        first = segments.begin()->getStart();
        last = segments.begin()->getEnd();

        sink.appendList();

        append(sink, first);
        append(sink, last);

        segments.erase(segments.begin());
      }

      // Find next closest segment
      float best = numeric_limits<float>::max();
      segments_t::iterator closestIt = segments.end();

      segments_t::iterator it;
      for (it = segments.begin(); it != segments.end(); it++) {
        float dist = distanceBetweenPoints(last, it->getStart());

        if (dist < best) {
          closestIt = it;
          best = dist;
        }
      }

      // Add it
      if (closestIt != segments.end()) {
        last = closestIt->getEnd();
        append(sink, last);
        segments.erase(closestIt);
      }

      // Detect end of loop
      if (segments.empty() || !distanceBetweenPoints(last, first)) {
        sink.endList();
        inLoop = false;
      }
    }

    sink.endList();
  }
}


STLModule::STLModule(TPLContext &ctx) : js::NativeModule("stl"), ctx(ctx) {}


void STLModule::define(js::Sink &exports) {
  exports.insert("open(path)", this, &STLModule::open);
  exports.insert("bounds(stl)", this, &STLModule::bounds);
  exports.insert("contour(stl, level, start, end, steps)", this,
                 &STLModule::contour);
}


void STLModule::open(const js::Value &args, js::Sink &sink) {
  // Read STL
  STL::Reader reader(ctx.relativePath(args.getString("path")));

  // Header
  string name;
  string hash;
  reader.readHeader(name, hash);

  sink.beginDict();
  sink.insert("name", name);
  sink.insert("hash", hash);

  // Facets
  sink.insertList("facets");

  while (reader.hasMore()) {
    Vector3F v[3];
    Vector3F normal;
    reader.readFacet(v[0], v[1], v[2], normal);

    sink.appendList();
    for (int i = 0; i < 3; i++) append(sink, v[i]);
    append(sink, normal);
    sink.endList();
  }

  sink.endList();
  sink.endDict();
}


void STLModule::bounds(const js::Value &args, js::Sink &sink) {
  SmartPointer<js::Value> facets = args.get("stl")->get("facets");
  Rectangle3F bounds;

  unsigned length = facets->length();
  for (unsigned i = 0; i < length; i++) {
    SmartPointer<js::Value> facet = facets->get(i);

    for (unsigned j = 0; j < 3; j++)
      bounds.add(toVector3F(*facet->get(j)));
  }

  sink.beginList();
  append(sink, bounds.getMin());
  append(sink, bounds.getMax());
  sink.endList();
}


/***
 * Creates a list of all contours at the level given by "level".
 *
 * It starts by loading each facet and looking at each edge of the triangle
 * to determine whether the facet crosses "level".
 *
 * If so, it determines the line segment that is formed by the intersection
 * of z = level and the facet.
 *
 * It uses normal to determine the direction of the line segment (i.e. which
 * is x1,y1 and which is x2,y2).  When traveling from x1,y1 to x2,y2, the normal
 * vector must always lean to the right to ensure that the polygon progresses
 * counterclockwise direction.
 *
 * After all line segments have been recorder, they are sorted to form closed
 * polygons.args.
 *
 * Each time a polygon closes, it is recorded into the return string
 * and a new polygon is started.
 *
 * If any polygons fail to close (within tolerance), an error is generated.
 *
 * The returned data is of the form:
 *
 *  [
 *    [{X: x1, Y: y1}, . . ., {X: xn, Y: yn}],
 *    . . .
 *    [{X: x1, Y: y1}, . . ., {X: xn, Y: yn}]
 *  ]
 */
void STLModule::contour(const js::Value &args, js::Sink &sink) {
  // Read facets
  vector<STL::Facet> facets;
  readFacets(facets, *args.get("stl")->get("facets"));

  // Process one level
  if (args.get("level")->isNumber())
    contourLevel(sink, facets, args.getNumber("level"));

  // Process multiple levels
  if (args.get("start")->isNumber() && args.get("end")->isNumber() &&
      args.get("steps")->isNumber()) {
    float start = args.getNumber("start");
    float end = args.getNumber("end");
    int steps = args.getInteger("steps");

    if (start == end || steps <= 1) return;

    float delta = (end - start) / (steps - 1);

    sink.beginList();
    for (int i = 0; i < steps; i++) {
      float level = start + delta * i;

      sink.appendDict();
      sink.insert("Z", level);
      sink.beginInsert("contours");
      contourLevel(sink, facets, level);
      sink.endDict();
    }
    sink.endList();
  }
}
