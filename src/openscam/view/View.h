/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#ifndef OPENSCAM_VIEW_H
#define OPENSCAM_VIEW_H

#include "ViewPort.h"
#include "ToolPathView.h"
#include "CuboidView.h"

#include <openscam/Geom.h>
#include <openscam/contour/Surface.h>
#include <openscam/value/ValueGroup.h>

#include <cbang/SmartPointer.h>
#include <cbang/time/Timer.h>

#include <vector>

namespace OpenSCAM {
  class View : public ViewPort {
    ValueGroup values;

  public:
    cb::SmartPointer<ToolPathView> path;
    cb::SmartPointer<CuboidView> workpiece;
    cb::SmartPointer<Surface> surface;

    enum {
      WIRE_FLAG                  = 1 << 0,
      SHOW_WORKPIECE_FLAG        = 1 << 2,
      SHOW_WORKPIECE_BOUNDS_FLAG = 1 << 3,
      SHOW_PATH_FLAG             = 1 << 4,
      SHOW_TOOL_FLAG             = 1 << 5,
      SHOW_SURFACE_FLAG          = 1 << 6,
      SHOW_BBTREE_FLAG           = 1 << 7,
      PLAY_FLAG                  = 1 << 8,
      LOOP_FLAG                  = 1 << 9,
    };

    unsigned flags;
    bool visible;

    double fps;
    unsigned speed;
    bool reverse;

    cb::Timer fpsTimer;
    unsigned frames;
    double frameTimes;

    double lastTime;

    View(ValueSet &valueSet);

    bool isFlagSet(unsigned flag) const {return flags & flag;}
    void setFlag(unsigned flag, bool on = true)
    {if (on) flags |= flag; else flags &= ~flag; values.updated();}
    void clearFlag(unsigned flag) {flags &= ~flag; values.updated();}
    void toggleFlag(unsigned flag) {flags ^= flag; values.updated();}

    void setSpeed(unsigned speed) {this->speed = speed;}
    void incSpeed();
    void decSpeed();
    void changeDirection() {reverse = !reverse; values.updated();}
    void frameTime(double delta);

    void setToolPath(const cb::SmartPointer<ToolPath> &toolPath);
    void setWorkpiece(const Rectangle3R &bounds);
    void setSurface(const cb::SmartPointer<Surface> &surface);
    double getTime() const {return path.isNull() ? 0 : path->getTime();}

    bool update();
  };
}

#endif // OPENSCAM_VIEW_H

