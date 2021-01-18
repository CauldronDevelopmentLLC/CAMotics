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

#include "ToolPathView.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>
#include <cbang/Catch.h>

#include <limits>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolPathView::ToolPathView(ValueSet &valueSet) : values(valueSet) {
  values.add("x", currentPosition.x());
  values.add("y", currentPosition.y());
  values.add("z", currentPosition.z());

  values.add("current_time", currentTime);
  values.add("total_time", this, &ToolPathView::getTotalTime);
  values.add("remaining_time", this, &ToolPathView::getRemainingTime);
  values.add("time_ratio", this, &ToolPathView::getTimeRatio);
  values.add("percent_time", this, &ToolPathView::getPercentTime);

  values.add("current_distance", currentDistance);
  values.add("total_distance", this, &ToolPathView::getTotalDistance);
  values.add("remaining_distance", this, &ToolPathView::getRemainingDistance);
  values.add("percent_distance", this, &ToolPathView::getPercentDistance);

  values.add("tool", this, &ToolPathView::getTool);
  values.add("feed", this, &ToolPathView::getFeed);
  values.add("speed", this, &ToolPathView::getSpeed);
  values.add("direction", this, &ToolPathView::getDirection);
  values.add("program_line", this, &ToolPathView::getProgramLine);

  setLight(false);
}


void ToolPathView::setPath(const SmartPointer<const GCode::ToolPath> &path) {
  this->path = path;

  currentMove = GCode::Move();
  dirty = true;

  if (path.isNull() || path->empty()) return;

  // Output stats
  const Rectangle3D &bbox = getBounds();
  Vector3D dims = bbox.getDimensions();
  LOG_INFO(1, "GCode::Tool Path bounds " << bbox << " dimensions " << dims);
}


void ToolPathView::setByRatio(double ratio) {
  if (byRemote || this->ratio != ratio) {
    this->ratio = ratio;
    byRemote = false;
    dirty = true;
  }
}


void ToolPathView::setByRemote(const Vector3D &position, unsigned line) {
  if (!byRemote || this->position != position || this->line != line) {
    byRemote = true;
    this->position = position;
    this->line = line;
    dirty = true;
  }
}


void ToolPathView::incTime(double amount) {
  double ratio = this->ratio + amount / getTotalTime();
  if (1 < ratio) ratio = 1;
  setByRatio(ratio);
}


void ToolPathView::decTime(double amount) {
  double ratio = this->ratio - amount / getTotalTime();
  if (ratio < 0) ratio = 0;
  setByRatio(ratio);
}


void ToolPathView::setShowIntensity(bool show) {
  if (show == showIntensity) return;
  showIntensity = show;
  dirty = true;
}


const char *ToolPathView::getDirection() const {
  if (!getMove().getSpeed()) return "Idle";
  return getMove().getSpeed() < 0 ? "Counterclockwise" : "Clockwise";
}


Color ToolPathView::getColor(GCode::MoveType type, double intensity) {
  switch (type) {
  case GCode::MoveType::MOVE_RAPID: return Color::RED;
  case GCode::MoveType::MOVE_CUTTING:
    return Color(0, intensity, 0.5 * (1 - intensity));
  case GCode::MoveType::MOVE_PROBE: return Color::BLUE;
  case GCode::MoveType::MOVE_DRILL: return Color::YELLOW;
  }
  THROW("Invalid move type " << type);
}


void ToolPathView::update() {
  if (!dirty) return;

  currentTime = 0;
  currentDistance = 0;
  currentPosition = byRemote ? position : Vector3D();
  currentLine = 0;
  currentMove = GCode::Move();

  vertices.clear();
  colors.clear();

  // Find maximum speed
  double maxSpeed = 0;
  if (showIntensity && !path.isNull())
    for (auto it = path->begin(); it != path->end(); it++)
      if (maxSpeed < fabs(it->getSpeed())) maxSpeed = fabs(it->getSpeed());

  // Find position on path
  if (!path.isNull())
    for (auto it = path->begin(); it != path->end(); it++) {
      GCode::Move move = *it;
      currentMove = move;

      const Vector3D &start = move.getStartPt();
      Vector3D end = move.getEndPt();
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
        double time = ratio * getTotalTime();
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

      // Store vertex and color data
      double s =
        (showIntensity && maxSpeed) ? fabs(move.getSpeed()) / maxSpeed : 1;
      Color color = getColor(move.getType(), s);

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

  values.updated();
  dirty = false;
}


void ToolPathView::glDraw(GLContext &gl) {
  if (path.isNull()) return;

  update();

  if (!numColors || !numVertices) return;

  // Setup color buffers
  if (!colors.empty()) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, colorVBuf.get());
    gl.glBufferData(GL_ARRAY_BUFFER, numColors * 3 * sizeof(float), &colors[0],
                    GL_STATIC_DRAW);
    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    colors.clear();
  }

  // Setup vertex buffers
  if (!vertices.empty()) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf.get());
    gl.glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float),
                    &vertices[0], GL_STATIC_DRAW);
    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertices.clear();
  }

  // Color
  gl.glEnableVertexAttribArray(GL_ATTR_COLOR);
  gl.glBindBuffer(GL_ARRAY_BUFFER, colorVBuf.get());
  gl.glVertexAttribPointer(GL_ATTR_COLOR, 3, GL_FLOAT, false, 0, 0);

  // Position
  gl.glEnableVertexAttribArray(GL_ATTR_POSITION);
  gl.glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf.get());
  gl.glVertexAttribPointer(GL_ATTR_POSITION, 3, GL_FLOAT, false, 0, 0);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Draw
  gl.glDrawArrays(GL_LINES, 0, numVertices);

  // Clean up
  gl.glDisableVertexAttribArray(GL_ATTR_POSITION);
  gl.glDisableVertexAttribArray(GL_ATTR_COLOR);
}
