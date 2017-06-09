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

#include "ToolPathView.h"

#include "GL.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

#include <limits>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathView::ToolPathView(ValueSet &valueSet) :
  values(valueSet), byRemote(true), ratio(1), line(0), totalTime(0),
  totalDistance(0), currentTime(0), currentDistance(0), currentLine(0),
  dirty(true), colorVBuf(0), vertexVBuf(0), numVertices(0), numColors(0),
  useVBOs(true) {

  values.add("x", currentPosition.x());
  values.add("y", currentPosition.y());
  values.add("z", currentPosition.z());

  values.add("current_time", currentTime);
  values.add("total_time", totalTime);
  values.add("remaining_time", this, &ToolPathView::getRemainingTime);
  values.add("time_ratio", this, &ToolPathView::getTimeRatio);
  values.add("percent_time", this, &ToolPathView::getPercentTime);

  values.add("current_distance", currentDistance);
  values.add("total_distance", totalDistance);
  values.add("remaining_distance", this, &ToolPathView::getRemainingDistance);
  values.add("percent_distance", this, &ToolPathView::getPercentDistance);

  values.add("tool", this, &ToolPathView::getTool);
  values.add("feed", this, &ToolPathView::getFeed);
  values.add("speed", this, &ToolPathView::getSpeed);
  values.add("direction", this, &ToolPathView::getDirection);
  values.add("program_line", this, &ToolPathView::getProgramLine);
}


ToolPathView::~ToolPathView() {
  if (colorVBuf) glDeleteBuffers(1, &colorVBuf);
  if (vertexVBuf) glDeleteBuffers(1, &vertexVBuf);
}


void ToolPathView::setPath(const SmartPointer<const GCode::ToolPath> &path,
                           bool withVBOs) {
  this->path = path;
  useVBOs = haveVBOs() && withVBOs;

  totalTime = 0;
  totalDistance = 0;
  currentMove = GCode::Move();

  if (!path.isNull()) {
    GCode::ToolPath::const_iterator it;
    for (it = path->begin(); it != path->end(); it++) {
      totalTime += it->getTime();
      totalDistance += it->getDistance();
    }
  }

  dirty = true;
  update();

  if (path.isNull() || path->empty()) return;

  // Output stats
  const cb::Rectangle3D &bbox = getBounds();
  cb::Vector3D dims = bbox.getDimensions();
  LOG_INFO(1, "GCode::Tool Path bounds " << bbox << " dimensions " << dims);
}


void ToolPathView::setByRatio(double ratio) {
  if (byRemote || this->ratio != ratio) {
    this->ratio = ratio;
    byRemote = false;
    dirty = true;
  }
}


void ToolPathView::setByRemote(const cb::Vector3D &position, unsigned line) {
  if (!byRemote || this->position != position || this->line != line) {
    byRemote = true;
    this->position = position;
    this->line = line;
    dirty = true;
  }
}


void ToolPathView::incTime(double amount) {
  double ratio = this->ratio + amount / totalTime;
  if (1 < ratio) ratio = 1;
  setByRatio(ratio);
}


void ToolPathView::decTime(double amount) {
  double ratio = this->ratio - amount / totalTime;
  if (ratio < 0) ratio = 0;
  setByRatio(ratio);
}


const char *ToolPathView::getDirection() const {
  if (!getMove().getSpeed()) return "Idle";
  return getMove().getSpeed() < 0 ? "Counterclockwise" : "Clockwise";
}


Color ToolPathView::getColor(GCode::MoveType type) {
  switch (type) {
  case GCode::MoveType::MOVE_RAPID:   return Color::RED;
  case GCode::MoveType::MOVE_CUTTING: return Color::GREEN;
  case GCode::MoveType::MOVE_PROBE:   return Color::BLUE;
  case GCode::MoveType::MOVE_DRILL:   return Color::YELLOW;
  }
  THROWS("Invalid move type " << type);
}


void ToolPathView::update() {
  if (!dirty) return;

  currentTime = 0;
  currentDistance = 0;
  currentPosition = byRemote ? position : cb::Vector3D();
  currentLine = 0;
  currentMove = GCode::Move();

  vertices.clear();
  colors.clear();

  // Find position on path
  GCode::ToolPath::const_iterator it;
  if (!path.isNull())
    for (it = path->begin(); it != path->end(); it++) {
      GCode::Move move = *it;
      currentMove = move;

      const cb::Vector3D &start = move.getStartPt();
      cb::Vector3D end = move.getEndPt();
      double moveTime = move.getTime();
      double moveDistance = move.getDistance();
      uint32_t moveLine = move.getLine() + 1; // EMC2 counts from zero
      bool partial = false;

      if (byRemote) {
        if (line < moveLine && !(byRemote && line < 1)) break; // Too far
        if (line == moveLine) {
          double distance = move.distance(position, end);

          // TODO should find the closest point on the closest move at this line
          if (distance < 0.00001) {
            double delta = start.distance(end);
            moveTime *= delta / moveDistance;
            moveDistance = delta;
            partial = true;
          }
        }
      } else {
        double time = ratio * totalTime;
        if (time < currentTime + moveTime) {
          double delta = time - currentTime;
          end = move.getPtAtTime(time);
          moveDistance *= delta / moveTime;
          moveTime = delta;
          partial = true;
        }
      }
      currentPosition = byRemote ? position : end;
      currentLine = moveLine;
      currentTime += moveTime;
      currentDistance += moveDistance;

      // Store GL data
      Color color = getColor(move.getType());
      for (unsigned i = 0; i < 3; i++) {
        colors.push_back(color[i]);
        vertices.push_back(start[i]);
      }

      for (unsigned i = 0; i < 3; i++) {
        colors.push_back(color[i]);
        vertices.push_back(end[i]);
      }

      if (partial) break;
    }

  numColors = colors.size() / 3;
  numVertices = vertices.size() / 3;

  // Setup GL Buffers
  // Colors
  if (useVBOs && !colors.empty()) {
    if (!colorVBuf) glGenBuffers(1, &colorVBuf);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBuf);
    glBufferData(GL_ARRAY_BUFFER, numColors * 3 * sizeof(float), &colors[0],
                 GL_STATIC_DRAW);
    colors.clear();
  }

  // Vertices
  if (useVBOs && !vertices.empty()) {
    if (!vertexVBuf) glGenBuffers(1, &vertexVBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), &vertices[0],
                 GL_STATIC_DRAW);
    vertices.clear();
  }

  values.updated();
  dirty = false;
}


void ToolPathView::draw() {
  if (path.isNull()) return;
  update();

  if (!numColors || !numVertices) return;

  if (useVBOs) {
    glBindBuffer(GL_ARRAY_BUFFER, colorVBuf);
    glColorPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

  } else {
    glColorPointer(3, GL_FLOAT, 0, &colors[0]);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  // Draw
  glLineWidth(1);
  glDrawArrays(GL_LINES, 0, numVertices);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}
