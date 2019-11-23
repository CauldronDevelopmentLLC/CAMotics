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


#include "ViewPort.h"
#include "ToolPathView.h"
#include "CuboidView.h"
#include "GLBox.h"
#include "ToolView.h"
#include "Mesh.h"
#include "MachineView.h"
#include "AABBView.h"

#include <camotics/contour/Surface.h>
#include <camotics/value/ValueGroup.h>
#include <camotics/machine/MachineModel.h>

#include <cbang/SmartPointer.h>
#include <cbang/time/Timer.h>

#include <vector>

namespace CAMotics {
  class MoveLookup;


  class View : public ViewPort {
    cb::SmartPointer<GLComposite> group;

    ValueGroup values;

    unsigned flags;

    unsigned speed = 1;
    bool reverse = false;

    double lastTime = 0;

  public:
    cb::SmartPointer<GLObject> axes;
    cb::SmartPointer<GLBox> bounds;
    cb::SmartPointer<ToolPathView> path;
    cb::SmartPointer<ToolView> tool;
    cb::SmartPointer<CuboidView> workpiece;
    cb::SmartPointer<Mesh> model;
    cb::SmartPointer<Lines> wireModel;
    cb::SmartPointer<Surface> surface;
    cb::SmartPointer<MoveLookup> moveLookup;
    cb::SmartPointer<AABBView> aabbView;
    cb::SmartPointer<MachineModel> machine;
    cb::SmartPointer<MachineView> machineView;

    cb::Rectangle3D workpieceBounds;
    cb::Rectangle3D surfaceBounds;

    bool surfaceChanged = false;
    bool machineChanged = false;
    bool moveLookupChanged = false;

    enum {
      WIRE_FLAG                  = 1 << 0,
      SHOW_WORKPIECE_FLAG        = 1 << 2,
      SHOW_WORKPIECE_BOUNDS_FLAG = 1 << 3,
      SHOW_PATH_FLAG             = 1 << 4,
      SHOW_TOOL_FLAG             = 1 << 5,
      SHOW_MACHINE_FLAG          = 1 << 6,
      SHOW_SURFACE_FLAG          = 1 << 7,
      SHOW_AXES_FLAG             = 1 << 8,
      SHOW_BBTREE_FLAG           = 1 << 9,
      BBTREE_LEAVES_FLAG         = 1 << 10,
      PLAY_FLAG                  = 1 << 11,
      TRANSLUCENT_SURFACE_FLAG   = 1 << 12,
      PATH_INTENSITY_FLAG        = 1 << 13,
    };

    View(ValueSet &valueSet);
    ~View();

    bool isFlagSet(unsigned flag) const {return flags & flag;}
    void setFlag(unsigned flag, bool on = true)
    {if (on) flags |= flag; else flags &= ~flag; values.updated();}
    void clearFlag(unsigned flag) {flags &= ~flag; values.updated();}
    void toggleFlag(unsigned flag) {flags ^= flag; values.updated();}

    unsigned getSpeed() const {return speed;}
    void setSpeed(unsigned speed) {this->speed = speed;}
    void incSpeed();
    void decSpeed();
    void setReverse(bool reverse) {this->reverse = reverse;}
    void changeDirection() {reverse = !reverse; values.updated();}

    void setToolPath(const cb::SmartPointer<GCode::ToolPath> &toolPath);
    void setWorkpiece(const cb::Rectangle3D &bounds);
    void setSurface(const cb::SmartPointer<Surface> &surface);
    void setMoveLookup(const cb::SmartPointer<MoveLookup> &moveLookup);
    void setMachine(const cb::SmartPointer<MachineModel> &machine);
    double getTime() const {return path.isNull() ? 0 : path->getTime();}

    bool update();
    void clear();

    void updateVisibility();
    void updateBounds();
    void updateTool();
    void loadWireModel();
    void updateModel();
    void updateMachine();
    void updateAABB();

    // From GLScene
    void glInit();
    void glDraw();
  };
}
