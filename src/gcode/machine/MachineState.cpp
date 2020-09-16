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

#include "MachineState.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>
#include <cbang/debug/Debugger.h>

#include <string.h> // For memset()

using namespace std;
using namespace cb;
using namespace GCode;


MachineState::MachineState() :
  started(false), feed(0), feedMode(UNITS_PER_MINUTE), speed(0),
  spinMode(REVOLUTIONS_PER_MINUTE), maxSpeed(0) {

  // Coordinate system
  set(CURRENT_COORD_SYSTEM, 1, NO_UNITS);

  // Tool
  set("_selected_tool", -1, NO_UNITS);
  set(TOOL_NUMBER, -1, NO_UNITS);

  // Units
  set("_metric", 1, NO_UNITS);
  set("_imperial", 0, NO_UNITS);

  // Arc error
  set("_max_arc_error", 0.01, METRIC);
}


void MachineState::start() {started = true;}


void MachineState::end() {
  if (!started) THROW("Machine not started");
  started = false;
}


void MachineState::setFeed(double feed) {
  this->feed = feed;
  set("_feed", feed, METRIC);
}


void MachineState::setSpeed(double speed) {
  this->speed = speed;
  set("_speed", speed, NO_UNITS);
}


void MachineState::setPathMode(path_mode_t mode, double motionBlending,
                               double naiveCAM) {
  set("_path_mode", (unsigned)mode, NO_UNITS);

  if (mode == CONTINUOUS_MODE) {
    set("_motion_blending_tolerance", motionBlending, METRIC);
    set("_naive_cam_tolerance", naiveCAM, METRIC);
  }
}


void MachineState::changeTool(unsigned tool) {
  set(TOOL_NUMBER, tool, NO_UNITS);
  set("_tool", tool, NO_UNITS);
}


Vector3D MachineState::getPosition(axes_t axes) const {
  switch (axes) {
  case XYZ: return position.getXYZ();
  case ABC: return position.getABC();
  case UVW: return position.getUVW();
  default: THROW("Invalid axes " << axes);
  }
}


void MachineState::setPosition(const Axes &position) {
  this->position.setFrom(position);
}


double MachineState::get(address_t addr, Units units) const {
  return MAX_ADDRESS <= addr ? 0 : params[addr].get(units);
}


void MachineState::set(address_t addr, double value, Units units) {
  if (addr < MAX_ADDRESS) params[addr].set(value, units);
}


bool MachineState::has(const string &name) const {
  return named.find(name) != named.end();
}


double MachineState::get(const string &name, Units units) const {
  auto it = named.find(name);
  return it == named.end() ? 0 : it->second.get(units);
}


void MachineState::set(const string &name, double value, Units units) {
  named[name] = Parameter(value, units);
}


void MachineState::clear(const string &name) {named.erase(name);}
