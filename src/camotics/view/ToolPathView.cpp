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
  values.add("program_file", this, &ToolPathView::getProgramFile);
  values.add("program_line", this, &ToolPathView::getProgramLine);

  setLight(false);
  setPickable(true);
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
  if (byLine || this->ratio != ratio) {
    this->ratio = ratio;
    byLine = false;
    dirty = true;
  }
}


void ToolPathView::setByLine(const string &filename, unsigned line,
  const Vector3D &position) {
  if (!byLine || this->filename != filename || this->line != line ||
      this->position != position) {
    this->filename = filename;
    this->line = line;
    this->position = position;
    byLine = true;
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
  currentPosition = (byLine && position.isReal()) ? position : Vector3D();
  currentLine = 0;
  currentFile = "";
  currentMove = GCode::Move();

  vertices.clear();
  colors.clear();
  picking.clear();

  // Find maximum speed
  double maxSpeed = 0;
  if (showIntensity && path.isSet())
    for (auto &move : *path)
      if (maxSpeed < fabs(move.getSpeed()))
        maxSpeed = fabs(move.getSpeed());

  // Find position on path
  if (!path.isNull()) {
    bool found = false;

    for (unsigned i = 0; i < path->size(); i++) {
      const GCode::Move &move = path->at(i);
      currentMove = move;

      const Vector3D &start = move.getStartPt();
      Vector3D end          = move.getEndPt();
      Vector3D mid;
      double moveTime       = move.getTime();
      double moveDistance   = move.getDistance();
      uint32_t moveLine     = move.getLine() + 1; // EMC2 counts from zero
      string moveFile;
      bool partial          = false;

      if (move.getFilename().isSet()) moveFile = *move.getFilename();

      if (!found) {
        if (byLine) {
          // Selection by line
          if ((filename.empty() || filename == moveFile) && line <= moveLine) {
            found = true;

            if (position.isReal()) {
              double distance = move.distance(position, mid);

              // TODO find the closest point on the closest move at this line
              if (distance < 0.00001) {
                double delta = start.distance(mid);
                moveTime *= delta / moveDistance;
                moveDistance = delta;
                partial = true;
              }
            }
          }

        } else {
          // Selection by time ratio
          double time = ratio * getTotalTime();

          if (time < currentTime + moveTime) {
            double delta = time - currentTime;
            mid = move.getPtAtTime(time);
            moveDistance *= delta / moveTime;
            moveTime = delta;
            partial = found = true;
          }
        }

        currentTime += moveTime;
        currentDistance += moveDistance;

        if (found) {
          currentLine = moveLine;
          currentFile = moveFile;
          if (!position.isReal()) currentPosition = partial ? mid : end;
        }
      }

      // Store vertex and color data
      double s =
        (showIntensity && maxSpeed) ? fabs(move.getSpeed()) / maxSpeed : 1;
      Color color = getColor(move.getType(), s);

      // Change color based on specified selection
      if (byLine && line == moveLine) color = Color::WHITE;
      else if (!partial && found) color *= Color(0.3, 0.3, 0.3);

      pushVertex(start, color, i);
      if (partial) {
        pushVertex(mid, color, i);
        color *= Color(0.3, 0.3, 0.3);
        pushVertex(mid, color, i);
      }
      pushVertex(end, color, i);
    }
  }

  numVertices = vertices.size() / 3;

  values.updated();
  dirty = false;
}


void ToolPathView::glDraw(GLContext &gl) {
  if (path.isNull()) return;

  update();

  if (!numVertices) return;

  // Setup picking buffers
  if (!picking.empty()) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, pickingVBuf.get());
    gl.glBufferData(GL_ARRAY_BUFFER, picking.size() * sizeof(float),
                    &picking[0], GL_STATIC_DRAW);
    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    picking.clear();
  }

  // Setup color buffers
  if (!colors.empty()) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, colorVBuf.get());
    gl.glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float),
                    &colors[0], GL_STATIC_DRAW);
    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    colors.clear();
  }

  // Setup vertex buffers
  if (!vertices.empty()) {
    gl.glBindBuffer(GL_ARRAY_BUFFER, vertexVBuf.get());
    gl.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                    &vertices[0], GL_STATIC_DRAW);
    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertices.clear();
  }

  // Picking
  gl.glEnableVertexAttribArray(GL_ATTR_PICKING);
  gl.glBindBuffer(GL_ARRAY_BUFFER, pickingVBuf.get());
  gl.glVertexAttribPointer(GL_ATTR_PICKING, 3, GL_FLOAT, false, 0, 0);

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
  gl.glDisableVertexAttribArray(GL_ATTR_PICKING);
}


void ToolPathView::pushVertex(
  const Vector3D &v, const Color &color, unsigned index) {

  // Convert index to RGB picking color
  Color pick = Color::fromIndex(index);

  for (unsigned i = 0; i < 3; i++) {
    colors.push_back(color[i]);
    picking.push_back(pick[i]);
    vertices.push_back(v[i]);
  }
}
