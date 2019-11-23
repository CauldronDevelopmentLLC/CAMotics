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

#pragma once

#include "GLObject.h"
#include "VBO.h"
#include "Color.h"

#include <gcode/ToolPath.h>
#include <camotics/value/ValueGroup.h>

#include <cbang/SmartPointer.h>
#include <cbang/StdTypes.h>

#include <vector>

namespace CAMotics {
  class ToolPathView : public GLObject {
    ValueGroup values;
    cb::SmartPointer<const GCode::ToolPath> path;

    bool byRemote = true;
    double ratio = 1;
    cb::Vector3D position;
    unsigned line = 0;

    double currentTime = 0;
    double currentDistance = 0;
    cb::Vector3D currentPosition;
    unsigned currentLine = 0;
    GCode::Move currentMove;

    bool dirty = true;
    bool showIntensity = false;

    std::vector<float> vertices;
    std::vector<float> colors;

    VBO colorVBuf;
    VBO vertexVBuf;
    unsigned numVertices = 0;
    unsigned numColors = 0;

  public:
    ToolPathView(ValueSet &valueSet);

    bool isEmpty() const {return path.isNull() || path->empty();}

    cb::SmartPointer<const GCode::ToolPath> getPath() const {return path;}
    void setPath(const cb::SmartPointer<const GCode::ToolPath> &path);

    cb::Rectangle3D getBounds() const
    {return path.isNull() ? cb::Rectangle3D() : path->getBounds();}

    void setByRatio(double ratio);
    void setByRemote(const cb::Vector3D &position, unsigned line);

    void incTime(double amount = 1);
    void decTime(double amount = 1);

    double getTime() const {return currentTime;}
    bool atStart() const {return getTime() == 0;}
    bool atEnd() const {return getTime() == getTotalTime();}
    double getRemainingTime() const {return getTotalTime() - currentTime;}
    double getTotalTime() const {return path.isNull() ? 0 : path->getTime();}
    double getTimeRatio() const {return currentTime / getTotalTime();}
    double getPercentTime() const {return getTimeRatio() * 100;}

    double getDistance() const {return currentDistance;}
    double getRemainingDistance() const
    {return getTotalDistance() - currentDistance;}
    double getTotalDistance() const
    {return path.isNull() ? 0 : path->getDistance();}
    double getPercentDistance() const
    {return currentDistance / getTotalDistance() * 100;}

    const cb::Vector3D &getPosition() const {return currentPosition;}
    unsigned getProgramLine() const {return currentLine;}
    const GCode::Move &getMove() const {return currentMove;}

    void setShowIntensity(bool show);

    unsigned getTool() const {return getMove().getTool();}
    double getFeed() const {return getMove().getFeed();}
    double getSpeed() const {return getMove().getSpeed();}
    const char *getDirection() const;

    Color getColor(GCode::MoveType type, double intensity);

    void update();

    // From GLObject
    void glDraw(GLContext &gl);
  };
}
