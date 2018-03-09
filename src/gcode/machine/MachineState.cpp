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

#include "MachineState.h"

#include <cbang/Exception.h>

#include <string.h> // For memset()

using namespace std;
using namespace cb;
using namespace GCode;


MachineState::MachineState() :
  started(false), feed(0), feedMode(UNITS_PER_MINUTE), speed(0),
  spinMode(REVOLUTIONS_PER_MINUTE), maxSpeed(0) {

  for (unsigned i = 0; i < AXES_COUNT; i++)
    matrices[i].toIdentity();

  // Init numbered parameters
  memset(params, 0, sizeof(params));

  // Coordinate system
  set(CURRENT_COORD_SYSTEM, 1, NO_UNITS);

  // Tool
  set("_selected_tool", -1, NO_UNITS);
  set(TOOL_NUMBER, -1, NO_UNITS);
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


void MachineState::changeTool(unsigned tool) {
  set(TOOL_NUMBER, tool, NO_UNITS);
  set("_tool", tool, NO_UNITS);
}


Vector3D MachineState::getPosition(axes_t axes) const {
  switch (axes) {
  case XYZ: return position.getXYZ();
  case ABC: return position.getABC();
  case UVW: return position.getUVW();
  default: THROWS("Invalid axes " << axes);
  }
}


const Matrix4x4D &MachineState::getMatrix(axes_t matrix) const {
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix " << matrix);
  return matrices[matrix];
}


void MachineState::setMatrix(const Matrix4x4D &m, axes_t matrix) {
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix " << matrix);
  matrices[matrix] = m;
}


double MachineState::get(address_t addr, Units units) const {
  if (MAX_ADDRESS <= addr) return 0;
  return convert(params[addr].units, units, params[addr].value);
}


void MachineState::set(address_t addr, double value, Units units) {
  if (addr < MAX_ADDRESS) params[addr] = {value, units};
}


bool MachineState::has(const string &name) const {
  return named.find(name) != named.end();
}


double MachineState::get(const string &name, Units units) const {
  named_t::const_iterator it = named.find(name);
  if (it == named.end()) return 0;
  return convert(it->second.units, units, it->second.value);
}


void MachineState::set(const string &name, double value, Units units) {
  named[name] = {value, units};
}
