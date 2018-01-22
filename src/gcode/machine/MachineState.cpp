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


void MachineState::reset() {
  started = false;
  feed = 0;
  feedMode = MM_PER_MINUTE;
  speed = 0;
  spinMode = REVOLUTIONS_PER_MINUTE;
  maxSpeed = 0;
  tool = -1;
  position = Axes();

  for (unsigned i = 0; i < AXES_COUNT; i++)
    matrices[i].toIdentity();

  location = LocationRange();

  // Init numbered parameters
  memset(params, 0, sizeof(params));
  set(CURRENT_COORD_SYSTEM, 1);
  set(TOOL_NUMBER, 1);
}


void MachineState::start() {started = true;}


void MachineState::end() {
  if (!started) THROW("Machine not started");
  started = false;
}


cb::Vector3D MachineState::getPosition(axes_t axes) const {
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


double MachineState::get(unsigned addr) const {
  return addr < MAX_ADDRESS ? params[addr] : params[0];
}


void MachineState::set(unsigned addr, double value) {
  if (addr < MAX_ADDRESS) params[addr] = value;
}


bool MachineState::has(const string &name) const {
  return named.find(name) != named.end();
}


double MachineState::get(const string &name) const {
  named_t::const_iterator it = named.find(name);
  return it == named.end() ? 0 : it->second;
}


void MachineState::set(const string &name, double value) {named[name] = value;}
