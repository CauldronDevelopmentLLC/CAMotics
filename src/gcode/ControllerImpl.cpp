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

#include "ControllerImpl.h"

#include "Codes.h"

#include <cbang/SStream.h>
#include <cbang/Math.h>
#include <cbang/Catch.h>
#include <cbang/log/Logger.h>

#include <limits>

#include <string.h> // For memset()


using namespace std;
using namespace cb;
using namespace GCode;


ControllerImpl::ControllerImpl(MachineInterface &machine,
                               const ToolTable &tools) : tools(tools) {
  state.units                      = this->machine.getUnits();
  state.plane                      = XY;
  state.cutterRadiusComp           = false;
  state.toolDiameter               = 0;
  state.toolOrientation            = 0;
  state.incrementalDistanceMode    = false;
  state.feedMode                   = UNITS_PER_MINUTE;
  state.coordSystem                = 1;
  state.toolLengthComp             = false;
  state.returnMode                 = RETURN_TO_R;
  state.spinMode                   = REVOLUTIONS_PER_MINUTE;
  state.arcIncrementalDistanceMode = true;
  state.latheDiameterMode          = true;
  state.pathMode                   = CONTINUOUS_MODE;
  state.feed                       = 0;
  state.speed                      = 0;
  state.spindleDir                 = DIR_OFF;
  state.mist                       = false;
  state.flood                      = false;
  state.speedOverride              = 0;
  state.feedOverride               = 0;
  state.adaptiveFeed               = false;
  state.feedHold                   = false;
  state.motionBlendingTolerance    = -1;
  state.naiveCamTolerance          = -1;
  state.moveInAbsoluteCoords       = false;

  this->machine.setParent(SmartPointer<MachineInterface>::Phony(&machine));

  memset(varValues, 0, sizeof(varValues));

  pushScope();
}


double ControllerImpl::getVar(char c) const {
  if (c < 'A' || 'Z' < c)
    THROW("Invalid var '" << String::escapeC(string(1, c)) << "'");
  return varValues[c - 'A'];
}


string ControllerImpl::getVarGroupStr(const char *group) const {
  string s;

  for (int i = 0; group[i]; i++)
    s += SSTR(' ' << group[i] << getVar(group[i]));

  return s;
}


Units ControllerImpl::getUnits() const {return machine.getUnits();}


void ControllerImpl::setUnits(Units units) {
  if (units == machine.getUnits()) return;

  switch (units) {
  case Units::NO_UNITS: THROW("Cannot set to NO_UNITS");

  case Units::METRIC:
    machine.setMetric();
    set("_metric",   1, NO_UNITS);
    set("_imperial", 0, NO_UNITS);
    position = position * 25.4;
    break;

  case Units::IMPERIAL:
    machine.setImperial();
    set("_metric",   0, NO_UNITS);
    set("_imperial", 1, NO_UNITS);
    position = position / 25.4;
    break;
  }

  state.units = units;
}


void ControllerImpl::setFeedMode(feed_mode_t mode) {
  state.feedMode = mode;
  machine.setFeedMode(mode);
}


void ControllerImpl::setSpindleDir(dir_t dir) {
  state.spindleDir = dir;
  setSpeed(state.speed);
}


void ControllerImpl::setSpinMode(spin_mode_t mode, double max) {
  state.spinMode = mode;
  state.spinMax = max;
  machine.setSpinMode(mode, max);
}


void ControllerImpl::setMistCoolant(bool enable) {
  state.mist = enable;
  machine.output(MachineEnum::MIST, enable);
  set("_mist", enable, NO_UNITS);
}


void ControllerImpl::setFloodCoolant(bool enable) {
  state.flood = enable;
  machine.output(MachineEnum::FLOOD, enable);
  set("_flood", enable, NO_UNITS);
}


void ControllerImpl::digitalOutput(unsigned index, bool enable,
                                   bool synchronized) {
  if (3 < index) {
    LOG_WARNING("Invalid digital output " << index);
    return;
  }

  machine.output((port_t)(DIGITAL_OUT_0 + index), enable);
}


void ControllerImpl::input(unsigned index, bool digital, input_mode_t mode,
                           double timeout) {
  if (3 < index) {
    LOG_WARNING("Invalid " << (digital ? "digital" : "analog") << " input "
                << index);
    return;
  }

  if (INPUT_LOW < mode) {
    LOG_WARNING("Invalid input mode " << mode);
    return;
  }

  if (timeout < 0) {
    LOG_WARNING("Invalid timeout " << timeout);
    return;
  }

  syncState = SYNC_INPUT; // Synchronize input result
  machine.input((port_t)((digital ? DIGITAL_IN_0 : ANALOG_IN_0) + index),
                mode, timeout);
}


void ControllerImpl::setPlane(plane_t plane) {
  if (VW < plane) THROW("Invalid plane: " << plane);
  state.plane = plane;
}


void ControllerImpl::setPathMode(path_mode_t mode, double motionBlending,
                                 double naiveCAM) {
  state.pathMode = mode;
  state.motionBlendingTolerance = motionBlending;
  state.naiveCamTolerance = naiveCAM;
  machine.setPathMode(mode, motionBlending, naiveCAM);
}


double ControllerImpl::getAxisCSOffset(char axis, unsigned cs) const {
  if (!cs) cs = get(CURRENT_COORD_SYSTEM);
  return get(COORD_SYSTEM_ADDR(cs, Axes::toIndex(axis)));
}


double ControllerImpl::getAxisToolOffset(char axis) const {
  return state.toolLengthComp ? get(TOOL_OFFSET_ADDR(Axes::toIndex(axis))) : 0;
}


double ControllerImpl::getAxisGlobalOffset(char axis) const {
  return get(GLOBAL_OFFSETS_ENABLED) ?
    get(GLOBAL_OFFSET_ADDR(Axes::toIndex(axis))) : 0;
}


double ControllerImpl::getAxisOffset(char axis) const {
  if (absoluteCoords) return 0; // Only set with G0 and G1 moves

  return getAxisCSOffset(axis) + getAxisToolOffset(axis) +
    getAxisGlobalOffset(axis);
}


double ControllerImpl::getAxisPosition(char axis) const {
  return getAxisAbsolutePosition(axis) - getAxisOffset(axis);
}


double ControllerImpl::getAxisAbsolutePosition(char axis) const {
  return position.get(axis);
}


void ControllerImpl::setAxisAbsolutePosition(char axis, double abs,
                                             Units units) {
  double scale = 1;
  if (units == METRIC && getUnits() == IMPERIAL) scale = 1.0 / 25.4;
  else if (units == IMPERIAL && getUnits() == METRIC) scale = 25.4;

  position.set(axis, abs * scale);

  double pos = abs - getAxisOffset(axis);
  set(CURRENT_COORD_ADDR(Axes::toIndex(axis)), pos, units);
  set("_" + string(1, tolower(axis)), pos, units);
}


Axes ControllerImpl::getAbsolutePosition() const {
  Axes pos;

  for (const char *axes = Axes::AXES; *axes; axes++)
    pos.set(*axes, getAxisAbsolutePosition(*axes));

  LOG_INFO(5, "Controller: Current absolute position is " << pos << "mm");

  return pos;
}


void ControllerImpl::setAbsolutePosition(const Axes &axes, Units units) {
  LOG_INFO(5, "Controller: Set absolute position to " << axes << units);

  for (const char *var = Axes::AXES; *var; var++)
    if (!isnan(axes.get(*var)))
      setAxisAbsolutePosition(*var, axes.get(*var), units);
}


Axes ControllerImpl::getNextAbsolutePosition(int vars, bool incremental) const {
  Axes position;

  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis)) {
      double pos = getVar(*axis);

      if (incremental) pos += getAxisAbsolutePosition(*axis);
      else pos += getAxisOffset(*axis);

      position.set(*axis, pos);

    } else position.set(*axis, getAxisAbsolutePosition(*axis));

  LOG_INFO(5, "Controller: Next absolute position is " << position << "mm");

  return position;
}


bool ControllerImpl::isPositionChanging(int vars, bool incremental) const {
  Axes nextPos = getNextAbsolutePosition(vars, state.incrementalDistanceMode);
  return nextPos != getAbsolutePosition();
}


void ControllerImpl::move(const Axes &pos, int axes, bool rapid) {
  machine.move(pos, axes, rapid);
  setAbsolutePosition(pos, getUnits());
}


void ControllerImpl::makeMove(int axes, bool rapid, bool incremental) {
  move(getNextAbsolutePosition(axes, incremental), axes, rapid);
}


void ControllerImpl::moveAxis(char axis, double value, bool rapid) {
  Axes pos = getAbsolutePosition();
  pos.set(axis, value + getAxisOffset(axis));
  move(pos, getVarType(axis), rapid);
}


void ControllerImpl::linear(int vars, bool rapid) {
  absoluteCoords = state.moveInAbsoluteCoords;
  makeMove(vars, rapid, state.incrementalDistanceMode);
  absoluteCoords = false;
}


void ControllerImpl::arc(int vars, bool clockwise) {
  // TODO Affected by cutter radius compensation
  // TODO Make sure this is correct for planes XZ and YZ

  const char *axes = getPlane().getAxes();
  if (state.plane == XZ) clockwise = !clockwise;

  // Compute start and end points
  Axes current(getAbsolutePosition());
  Axes target(getNextAbsolutePosition(vars, state.incrementalDistanceMode));
  Vector2D start(current.get(axes[0]), current.get(axes[1]));
  Vector2D finish(target.get(axes[0]), target.get(axes[1]));
  Vector2D center;
  double radius;

  if (VT_R & vars) {
    radius = getVar('R');

    // If radius is less than half the distance then the arc is impossible
    double a = start.distance(finish) / 2;
    if (fabs(radius) < a - 0.00001) {
      LOG_WARNING("Impossible radius format arc, replacing with line segment, "
                  "radius=" << fabs(radius) << " distance/2=" << a);
      move(target, vars, false);
      return;
    }

    // Compute arc center
    Vector2D m((start + finish) / 2);
    Vector2D E(start.y() - finish.y(), finish.x() - start.x());
    double d = (finish - start).length() / 2;
    double l = radius * radius - d * d;

    // Handle possible small negative caused by rounding errors
    l = l < 0 ? 0 : sqrt(l);

    if (!clockwise) l = -l;
    if (0 < radius) l = -l;

    center = m + E.normalize() * l;

    static bool warned = false;
    if (!warned) LOG_WARNING("Radius format arcs are discouraged");
    warned = true;

  } else {
    // Get arc center
    const char *offsets = getPlane().getOffsets();
    Vector2D offset;

    if (state.arcIncrementalDistanceMode) {
      offset =
        Vector2D((vars & getVarType(offsets[0])) ? getVar(offsets[0]) : 0,
                 (vars & getVarType(offsets[1])) ? getVar(offsets[1]) : 0);
      center = start + offset;

    } else center = Vector2D(getVar(offsets[0]) - getAxisOffset(axes[0]),
                             getVar(offsets[1]) - getAxisOffset(axes[1]));

    // Check that the radius matches
    radius = start.distance(center);
    double radiusDiff = fabs(radius - finish.distance(center));
    if ((machine.isImperial() && 0.0005 < radiusDiff) ||
        (machine.isMetric() && 0.005 < radiusDiff)) {
      LOG_WARNING("Arc radiuses differ by " << radiusDiff << "mm");
      LOG_DEBUG(1, " center=" << center << " start=" << start
                << " finish=" << finish << " offset=" << offset);
    }
  }

  // Compute angle
  double startAngle = (start - center).angleBetween(Vector2D(1, 0));
  double finishAngle = (finish - center).angleBetween(Vector2D(1, 0));
  double angle = finishAngle - startAngle;
  if (0 <= angle) angle -= 2 * M_PI;
  if (!clockwise) angle += 2 * M_PI;
  if (!angle) angle = 2 * M_PI;

  if ((VT_P & vars) && 1 < getVar('P'))
    angle += M_PI * 2 * (getVar('P') - 1) * (angle < 0 ? -1 : 1);

  LOG_DEBUG(4, "Arc angle=" << angle << " startAngle=" << startAngle
            << " finishAngle=" << finishAngle << " start=" << start
            << " finish=" << finish << " center=" << center
            << " radius=" << radius);

  // Do arc
  double deltaZ = target.get(axes[2]) - current.get(axes[2]);
  Axes offset;
  offset.set(axes[0], center[0] - start[0]);
  offset.set(axes[1], center[1] - start[1]);
  offset.set(axes[2], deltaZ);

  machine.arc(offset.getXYZ(), target.getXYZ(), -angle, state.plane);

  setAbsolutePosition(target, getUnits());
  LOG_INFO(3, "Controller: Arc");
}


void ControllerImpl::straightProbe(int vars, bool towardWorkpiece,
                                   bool signalError) {
  if (!isPositionChanging(vars, state.incrementalDistanceMode))
    THROW("Probe target position is same as current position");

  syncState = SYNC_PROBE; // Synchronize with found position
  machine.seek(PROBE, towardWorkpiece, signalError);
  makeMove(vars, false, state.incrementalDistanceMode);

  LOG_INFO(3, "Controller: straight probe "
           << (towardWorkpiece ? "toward" : "away from") << " workpiece"
           << (signalError ? " with error signal" : ""));
}


void ControllerImpl::seek(int vars, bool active, bool error) {
  port_t port;

  if (vars & VT_P) port = (port_t)(unsigned)round(getVar('P'));
  else {
    char targetAxis = 0;
    bool seekMin;

    for (const char *axes = Axes::AXES; *axes; axes++)
      if (vars & getVarType(*axes)) {
        if (targetAxis) THROW("Multiple axes in seek");
        targetAxis = *axes;
        seekMin = (0 < getVar(*axes)) ^ active;
      }

    switch (targetAxis) {
    case 'X': port = seekMin ? X_MIN : X_MAX; break;
    case 'Y': port = seekMin ? Y_MIN : Y_MAX; break;
    case 'Z': port = seekMin ? Z_MIN : Z_MAX; break;
    case 'A': port = seekMin ? A_MIN : A_MAX; break;
    case 'B': port = seekMin ? B_MIN : B_MAX; break;
    case 'C': port = seekMin ? C_MIN : C_MAX; break;
    case 'U': port = seekMin ? U_MIN : U_MAX; break;
    case 'V': port = seekMin ? V_MIN : V_MAX; break;
    case 'W': port = seekMin ? W_MIN : W_MAX; break;
    default: THROW("Seek requires axis");
    }
  }

  if (!isPositionChanging(vars, true))
    THROW("Seek target position is same as current position");

  syncState = SYNC_SEEK; // Synchronize with found position
  machine.seek(port, active, error);
  makeMove(vars, false, true);
}


void ControllerImpl::drill(int vars, bool dwell, bool feedOut,
                           bool spindleStop) {
  Plane plane = getPlane();
  unsigned xyVars = vars & (plane.getXVarType() | plane.getYVarType());
  unsigned zVar = plane.getZVarType();
  double r = getVar('R');
  unsigned L = (vars & VT_L) ? (unsigned)getVar('L') : 1;

  double zClear = 0;
  switch (state.returnMode) {
  case RETURN_TO_R: zClear = r; break;
  case RETURN_TO_OLD_Z: {
    double oldZ = getAxisPosition(plane.getZAxis());
    zClear = (r < oldZ) ? oldZ : r;
    break;
  }
  }

  for (unsigned i = 0; i < L; i++) {
    // Z is below R
    double z = getAxisPosition(plane.getZAxis());
    if (!i && z < r) moveAxis(plane.getZAxis(), r, true);

    // Traverse to XY
    makeMove(xyVars, true, state.incrementalDistanceMode);

    // Move to Z if above
    z = getAxisPosition(plane.getZAxis());
    if (z != r) moveAxis(plane.getZAxis(), r, true);

    // Drill
    makeMove(zVar, false, state.incrementalDistanceMode);

    // Dwell
    if (dwell) this->dwell(getVar('P'));

    // Stop Spindle
    dir_t spindleDir = state.spindleDir;
    if (spindleStop) setSpindleDir(DIR_OFF);

    // Retract
    moveAxis(plane.getZAxis(), zClear, !feedOut);

    // Restart Spindle
    if (spindleStop) setSpindleDir(spindleDir);
  }
}


void ControllerImpl::dwell(double seconds) {machine.dwell(seconds);}


void ControllerImpl::pause(pause_t type) {
  syncState = SYNC_PAUSE;
  machine.pause(type);
}


void ControllerImpl::setTools(int vars, bool relative, bool cs9) {
  Tool &tool = getTool(getVar('P'));

  for (const char *v = Tool::VARS; *v; v++)
    if (vars & getVarType(*v)) {
      double offset = getVar(*v);

      // G10 L10 & L11
      if (relative && (getVarType(*v) & VT_AXIS)) {
        unsigned cs = cs9 ? 9 : get(CURRENT_COORD_SYSTEM);

        offset = getAxisAbsolutePosition(*v) - getAxisCSOffset(*v, cs) -
          (cs9 ? 0 : getAxisGlobalOffset(*v)) - offset;
      }

      tool.set(*v, offset);
    }

  LOG_INFO(3, "Controller: Set Tool Table" << getVarGroupStr("PRXYZABCUVWIJQ"));
}


void ControllerImpl::toolChange() {
  int tool = get("_selected_tool");
  if (tool < 0) THROW("No tool selected");
  machine.changeTool(tool);
  LOG_INFO(3, "Controller: Tool change " << tool);
}


void ControllerImpl::loadToolOffsets(unsigned number, bool add) {
  try {
    const Tool &tool = getTool(number);

    for (const char *axis = Axes::AXES; *axis; axis++) {
      address_t addr = TOOL_OFFSET_ADDR(Axes::toIndex(*axis));
      double offset = tool.get(*axis) + (add ? get(addr, getUnits()) : 0);
      set(addr, offset, getUnits());
    }

    state.toolLengthComp = true;
  } CATCH_WARNING; // Tool may not exist
}


void ControllerImpl::loadToolVarOffsets(int vars) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis)) {
      address_t addr = TOOL_OFFSET_ADDR(Axes::toIndex(*axis));
      set(addr, getVar(*axis), getUnits());
    }

  state.toolLengthComp = true;
}


void ControllerImpl::storePredefined(bool first) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    set(PREDEFINED_ADDR(first, Axes::toIndex(*axis)),
        getAxisAbsolutePosition(*axis), getUnits());
}


void ControllerImpl::loadPredefined(bool first, int vars) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis)) {
      address_t addr = PREDEFINED_ADDR(first, Axes::toIndex(*axis));
      setAxisAbsolutePosition(*axis, get(addr), getUnits());
    }
}


void ControllerImpl::setCoordSystem(unsigned cs) {
  state.coordSystem = cs;
  set(CURRENT_COORD_SYSTEM, cs, NO_UNITS);
}


void ControllerImpl::setCoordSystemOffsets(int vars, bool relative) {
  unsigned cs = getVar('P');
  if (9 < cs) THROW("Invalid coordinate system number " << cs);
  if (!cs) cs = get(CURRENT_COORD_SYSTEM);

  if (vars & VarTypes::VT_R) {
    if (relative) LOG_WARNING("R not allowed in G10 L20");
    else {
      address_t addr = COORD_SYSTEM_ADDR(cs, COORD_SYSTEM_ROTATION_MEMBER);
      set(addr, getVar('R'), NO_UNITS);
    }
  }

  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis)) {
      address_t addr = COORD_SYSTEM_ADDR(cs, Axes::toIndex(*axis));
      double offset = getVar(*axis);

      if (relative)
        offset = getAxisPosition(*axis) + getAxisCSOffset(*axis) - offset;

      set(addr, offset, getUnits());
    }
}


void ControllerImpl::setAxisGlobalOffset(char axis, double offset) {
  set(GLOBAL_OFFSET_ADDR(Axes::toIndex(axis)), offset, getUnits());
}


void ControllerImpl::setGlobalOffsets(int vars, bool relative) {
  set(GLOBAL_OFFSETS_ENABLED, 1, NO_UNITS);

  // Make the current point have the coordinates specified, without motion
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (getVarType(*axis) & vars) {
      double offset = getVar(*axis);
      if (relative)
        offset = getAxisPosition(*axis) + getAxisGlobalOffset(*axis) - offset;
      setAxisGlobalOffset(*axis, offset);
    }
}


void ControllerImpl::resetGlobalOffsets(bool clear) {
  set(GLOBAL_OFFSETS_ENABLED, 0, NO_UNITS);

  if (clear)
    for (unsigned axis = 0; axis < 9; axis++)
      set(GLOBAL_OFFSET_ADDR(axis), 0, getUnits());
}


void ControllerImpl::restoreGlobalOffsets() {
  set(GLOBAL_OFFSETS_ENABLED, 1, NO_UNITS);
}


void ControllerImpl::updateOffsetParams() {
  if (!offsetParamChanged) return;
  offsetParamChanged = false;

  for (const char *axis = Axes::AXES; *axis; axis++) {
    string name = "_offset_" + string(1, tolower(*axis));
    double offset = getAxisOffset(*axis);

    if (!has(name) || offset != get(name)) {
      set(name, offset, getUnits());

      double position = getAxisPosition(*axis);
      address_t addr = CURRENT_COORD_ADDR(Axes::toIndex(*axis));
      set(addr, position, getUnits());
      set("_" + string(1, tolower(*axis)), position, getUnits());
    }
  }

  // Apply rotation
  unsigned cs = get(CURRENT_COORD_SYSTEM);
  address_t addr = COORD_SYSTEM_ADDR(cs, COORD_SYSTEM_ROTATION_MEMBER);
  double r = get(addr) * M_PI / 180;
  Transform &t = machine.getTransforms().get(XYZ).pull();
  t.rotate(r, Vector3D(0, 0, 1), Vector3D(0, 0, 0));
}


void ControllerImpl::setHomed(int vars, bool homed) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (getVarType(*axis) & vars) {
      string name = SSTR("_" << (char)tolower(*axis) << "_homed");
      set(name, homed, NO_UNITS);

      if (homed) {
        string name = SSTR("_" << (char)tolower(*axis) << "_home");
        set(name, getVar(*axis), getUnits());
        setAxisAbsolutePosition(*axis, getVar(*axis), getUnits());
        setAxisGlobalOffset(*axis, 0);
      }
    }

  // Notify machine that axis positions have changed
  if (homed) machine.setPosition(getAbsolutePosition());
}


void ControllerImpl::setCutterRadiusComp(int vars, bool left, bool dynamic) {
  LOG_WARNING("Cutter radius compensation not implemented"); // TODO
  if (getCurrentTool() < 0) {
    LOG_ERROR("Tool not set, cannot set cutter radius compensation");
    return;
  }

  // NOTE, Fanuc may use ``D`` or ``H`` for the same purpose.

  try {
    if (dynamic) {
      state.toolDiameter = (left ? 1 : -1) * getVar('D');
      if (vars & VT_L) state.toolOrientation = getVar('L');
      else state.toolOrientation = 0;

    } else {
      state.toolOrientation = 0;

      if (!(vars & VT_D))
        state.toolDiameter = getTool(getCurrentTool()).getRadius() * 2;
      else if (!getVar('D')) state.toolDiameter = 0;
      else state.toolDiameter = getTool(getVar('D')).getRadius() * 2;
    }

    set(TOOL_DIAMETER, state.toolDiameter, getUnits());
    set(TOOL_ORIENTATION, state.toolOrientation, NO_UNITS);
    state.cutterRadiusComp = true;

  } CATCH_WARNING; // Tool may not exist
}


void ControllerImpl::end() {
  // TODO replace this with GCode override
  // See http://linuxcnc.org/docs/html/gcode/m-code.html#mcode:m2-m30

  // Origin offsets are set to the default (G54)
  setCoordSystem(1);

  // Selected plane is set to XY plane (G17)
  setPlane(XY);

  // Distance mode is set to absolute mode (G90)
  state.incrementalDistanceMode = false;

  // Feed rate mode is set to units per minute (G94)
  setFeedMode(UNITS_PER_MINUTE);

  // TODO Feed and speed overrides are set to on (M48)

  // Cutter compensation is turned off (G40)
  state.cutterRadiusComp = false;

  // The spindle is stopped (M5)
  setSpindleDir(DIR_OFF);

  // The current motion mode is set to feed (G1)
  setCurrentMotionMode(10);

  // Coolant is turned off (M9)
  setMistCoolant(false);
  setFloodCoolant(false);

  // Update offsets after changes above
  updateOffsetParams();

  throw EndProgram();
}


void ControllerImpl::stop() {
  // The spindle is stopped (M5)
  setSpindleDir(DIR_OFF);

  // Coolant is turned off (M9)
  setMistCoolant(false);
  setFloodCoolant(false);
}


double ControllerImpl::get(address_t addr, Units units) const {
  return machine.get(addr, units);
}


void ControllerImpl::set(address_t addr, double value, Units units) {
  machine.set(addr, value, units);

  // Check if offsets have changed
  if (GLOBAL_OFFSETS_ENABLED <= addr && addr <= CS9_ROTATION)
    offsetParamChanged = true;
}


double ControllerImpl::get(const string &name, Units units) const {
  return machine.get(name, units);
}


void ControllerImpl::set(const string &name, double value, Units units) {
  machine.set(name, value, units);
}


void ControllerImpl::saveModalState(bool autoRestore) {
  if (autoRestore && stateStack.size() == 1)
    LOG_WARNING("Cannot autorestore modal state from top scope");

  state.autoRestore = autoRestore;
  stateStack.back() = new state_t(this->state);
}


void ControllerImpl::clearSavedModalState() {stateStack.back().release();}


void ControllerImpl::restoreModalState() {
  if (stateStack.back().isNull()) {
    LOG_ERROR("Cannot restore modal state when not previously saved at this "
              "scope");
    return;
  }

  state = *stateStack.back();

  setUnits(state.units);
  set(TOOL_DIAMETER, state.toolDiameter, getUnits());
  set(TOOL_ORIENTATION, state.toolOrientation, NO_UNITS);
  setFeedMode(state.feedMode);
  setCoordSystem(state.coordSystem);
  setSpinMode(state.spinMode, state.spinMax);
  setFeed(state.feed);
  setSpeed(state.speed);
  setMistCoolant(state.mist);
  setFloodCoolant(state.flood);
  setPathMode(state.pathMode, state.motionBlendingTolerance,
              state.naiveCamTolerance);
}


void ControllerImpl::message(const string &text) {machine.message(text);}
double ControllerImpl::get(address_t addr) const {return get(addr, getUnits());}


void ControllerImpl::set(address_t addr, double value) {
  set(addr, value, NO_UNITS);
}


bool ControllerImpl::has(const string &name) const {return machine.has(name);}


double ControllerImpl::get(const string &name) const {
  return get(name, getUnits());
}


void ControllerImpl::set(const string &name, double value) {
  set(name, value, NO_UNITS);
}


void ControllerImpl::clear(const string &name) {machine.clear(name);}


void ControllerImpl::setVar(char c, double value) {
  if (c < 'A' || 'Z' < c)
    THROW("Invalid var '" << String::escapeC(string(1, c)) << "'");
  varValues[c - 'A'] = value;
}


void ControllerImpl::setCurrentMotionMode(unsigned mode) {
  currentMotionMode = mode;
}


void ControllerImpl::synchronize(double result) {
  if (syncState == SYNC_NONE) THROW("Not synchronizing");

  switch (syncState) {
  case SYNC_NONE: break;

  case SYNC_SEEK:
  case SYNC_PROBE:
    // Set PROBE_SUCCESS and probed position in PROBE_RESULT_X to W
    set(PROBE_SUCCESS, result, NO_UNITS);

    for (const char *axis = Axes::AXES; *axis; axis++)
      set(PROBE_RESULT_ADDR(Axes::toIndex(*axis)),
          getAxisAbsolutePosition(*axis), getUnits());
    break;

  case SYNC_INPUT: set(USER_INPUT, result, NO_UNITS); break;
  case SYNC_PAUSE: break;
  }

  syncState = SYNC_NONE;
}


void ControllerImpl::setLocation(const LocationRange &location) {
  machine.setLocation(location);
}


void ControllerImpl::setFeed(double feed) {
  state.feed = feed;
  machine.setFeed(feed);
}


void ControllerImpl::setSpeed(double speed) {
  state.speed = speed;

  double mspeed;

  switch (state.spindleDir) {
  case DIR_OFF: mspeed = 0; break;
  case DIR_CLOCKWISE: mspeed = speed; break;
  case DIR_COUNTERCLOCKWISE: mspeed = -speed; break;
  default: THROW("Invalid spindle direction");
  }

  machine.setSpeed(mspeed);
}


void ControllerImpl::setTool(unsigned tool) {
  set("_selected_tool", tool, NO_UNITS);
}


void ControllerImpl::pushScope() {stateStack.push_back(0);}


void ControllerImpl::popScope() {
  if (!stateStack.back().isNull() && stateStack.back()->autoRestore)
    restoreModalState();
  stateStack.pop_back();
}


void ControllerImpl::startBlock() {
  if (syncState != SYNC_NONE) {
    LOG_WARNING("Position after synchronized command unknown in simulator.");
    syncState = SYNC_NONE;
  }
  state.moveInAbsoluteCoords = false;
}


bool ControllerImpl::execute(const Code &code, int vars) {
  bool implemented = true;

  switch (code.type) {
  case 'G':
    switch (code.number) {
    case 0:  linear(vars, true);  break;
    case 10: linear(vars, false); break;

    case 20: arc(vars, true);     break;
    case 30: arc(vars, false);    break;

    case 40: dwell(getVar('P'));  break;

    case 50: implemented = false; break; // TODO Cubic B-spline
    case 51: implemented = false; break; // TODO Quadratic B-spline
    case 52: implemented = false; break; // TODO NURBS Block Start
    case 53: implemented = false; break; // TODO NURBS Block End

    case 70: state.latheDiameterMode = true;  break;
    case 80: state.latheDiameterMode = false; break; // Radius mode

    case 100:
      if (VT_L & vars)
        switch ((unsigned)getVar('L')) {
        case 1:  setTools(vars, false, false); break;
        case 2:  setCoordSystemOffsets(vars, false); break;
        case 10: setTools(vars, true, false); break;
        case 11: setTools(vars, true, true); break;
        case 20: setCoordSystemOffsets(vars, true); break;
        default: LOG_WARNING("G10 with unsupported L code " << getVar('L'));
        }

      else LOG_WARNING("G10 with out L code");
      break;

    case 170: setPlane(XY); break;
    case 171: setPlane(UV); break;
    case 180: setPlane(XZ); break;
    case 181: setPlane(UW); break;
    case 190: setPlane(YZ); break;
    case 191: setPlane(VW); break;

    case 200: setUnits(Units::IMPERIAL); break;
    case 210: setUnits(Units::METRIC);   break;

    case 280: case 300:
      // See RS274/NGC v3.0 section 3.5.8
      // Note, at the time of this writing LinuxCNC says only the specified axes
      // should move but this contradicts RS274/NGC.

      // Intermediate move
      if (vars & VT_AXIS) makeMove(vars, true, state.incrementalDistanceMode);

      // Move all axes to predefined position
      loadPredefined(code.number == 280, VT_AXIS);
      move(getAbsolutePosition(), VT_AXIS, true);
      break;

    case 281: storePredefined(true);  break;
    case 282: setHomed(vars, false);  break;
    case 283: setHomed(vars, true);   break;
    case 301: storePredefined(false); break;

    case 330: implemented = false; break; // TODO Spindle synchronized motion
    case 331: implemented = false; break; // TODO Rigid tapping

    case 382: straightProbe(vars, true,  true);  break;
    case 383: straightProbe(vars, true,  false); break;
    case 384: straightProbe(vars, false, true);  break;
    case 385: straightProbe(vars, false, false); break;

    case 386: seek(vars, true,  true);  break;
    case 387: seek(vars, true,  false); break;
    case 388: seek(vars, false, true);  break;
    case 389: seek(vars, false, false); break;

    case 400: state.cutterRadiusComp = false; break;
    case 410: setCutterRadiusComp(vars, true, false);  break;
    case 411: setCutterRadiusComp(vars, true, true);   break;
    case 420: setCutterRadiusComp(vars, false, false); break;
    case 421: setCutterRadiusComp(vars, false, true);  break;

    case 430:
      if (getCurrentTool() < 0) {
        LOG_ERROR("Tool not set, cannot load tool offsets");
        break;
      }
      loadToolOffsets((vars & VT_H) ? getVar('H') : getCurrentTool(), false);
      break;

    case 431: loadToolVarOffsets(vars); break;
    case 432: loadToolOffsets(getVar('H'), true); break;
    case 490: state.toolLengthComp = false; break;

    case 520: setGlobalOffsets(vars, false); break;
    case 530: state.moveInAbsoluteCoords = true;  break;
    case 540: setCoordSystem(1); break;
    case 550: setCoordSystem(2); break;
    case 560: setCoordSystem(3); break;
    case 570: setCoordSystem(4); break;
    case 580: setCoordSystem(5); break;
    case 590: setCoordSystem(6); break;
    case 591: setCoordSystem(7); break;
    case 592: setCoordSystem(8); break;
    case 593: setCoordSystem(9); break;

    case 610: setPathMode(EXACT_PATH_MODE); break;
    case 611: setPathMode(EXACT_STOP_MODE); break;

    case 640: {
      double p = vars & VT_P ? getVar('P') : -1;
      double q = vars & VT_Q ? getVar('Q') : p;
      setPathMode(CONTINUOUS_MODE, p, q);
      break;
    }

    case 700: implemented = false; break; // Mach3/4 Inches mode
    case 710: implemented = false; break; // Mach3/4 MM mode

    case 730: implemented = false; break; // Drill cycle w/ chip breaking
    case 760: implemented = false; break; // Thread cycle

    case 800:                                   break; // Cancel canned cycle
    case 810: drill(vars, false, false, false); break; // Drill cycle
    case 820: drill(vars, true, false, false);  break; // Drill cycle w/ dwell
    case 830: // TODO Peck drill
      drill(vars, false, false, false); // Better than nothing
      implemented = false;
      break;
    case 840: // TODO Right-hand tap
      implemented = false;
      break;
    case 850: drill(vars, false, true, false);  break; // No dwell, feed out
    case 860: drill(vars, false, false, true);  break; // Spin stop rapid out
    case 870: // TODO Back boring
      drill(vars, false, false, false); // Better than nothing
      implemented = false;
      break;
    case 880: // TODO Spin stop manual out
      drill(vars, false, false, false); // Better than nothing
      implemented = false;
      break;
    case 890: drill(vars, true, true, false);   break; // Dwell, feed out

    case 900: state.incrementalDistanceMode    = false; break;
    case 901: state.arcIncrementalDistanceMode = false; break;
    case 910: state.incrementalDistanceMode    = true;  break;
    case 911: state.arcIncrementalDistanceMode = true;  break;

    case 920: setGlobalOffsets(vars, true); break;
    case 921: resetGlobalOffsets(true);     break;
    case 922: resetGlobalOffsets(false);    break;
    case 923: restoreGlobalOffsets();       break;

    case 930: setFeedMode(INVERSE_TIME);         break;
    case 940: setFeedMode(UNITS_PER_MINUTE);     break;
    case 950: setFeedMode(UNITS_PER_REVOLUTION); break;

      // NOTE: The spindle modes must be accompanied by a speed
    case 960:
      setSpinMode(CONSTANT_SURFACE_SPEED, (vars & VT_D) ? getVar('D') : 0);
      break;

    case 970: setSpinMode(REVOLUTIONS_PER_MINUTE); break;

    case 980: state.returnMode = RETURN_TO_R;     break;
    case 990: state.returnMode = RETURN_TO_OLD_Z; break;

    default: implemented = false;
    }
    break;

  case 'M':
    switch (code.number) {
    case 0:  pause(PAUSE_PROGRAM);                break; // Pause
    case 10: pause(PAUSE_OPTIONAL);               break; // Optional pause
    case 20: end();                               break; // End program
    case 30: setSpindleDir(DIR_CLOCKWISE);        break;
    case 40: setSpindleDir(DIR_COUNTERCLOCKWISE); break;
    case 50: setSpindleDir(DIR_OFF);              break;
    case 60: toolChange();                        break; // Manual Tool Change
    case 70: setMistCoolant(true);                break;
    case 71: setMistCoolant(false);               break;
    case 80: setFloodCoolant(true);               break;
    case 81: setFloodCoolant(false);              break;
    case 90:
      setMistCoolant(false);
      setFloodCoolant(false);
      break;

    case 300: end(); break; // TODO Exchange pallet shuttles and end program

    case 600: pause(PAUSE_PALLET_CHANGE); break; // Pallet change pause

    case 620: digitalOutput(getVar('P'), true,  true);  break;
    case 630: digitalOutput(getVar('P'), false, true);  break;
    case 640: digitalOutput(getVar('P'), true,  false); break;
    case 650: digitalOutput(getVar('P'), false, false); break;

    case 660:
      input((vars & VT_P) ? getVar('P') : getVar('E'), vars & VT_P,
            (input_mode_t)(unsigned)((vars & VT_L) ? getVar('L') : 0),
            (vars & VT_Q) ? getVar('Q') : 0);
      break;

    case 700: saveModalState(false);  break;
    case 710: clearSavedModalState(); break;
    case 720: restoreModalState();    break;
    case 730: saveModalState(true);   break;

      // TODO the following M-Codes
    case 190: // Orient spindle
    case 480: case 490: // Speed and feed override control
    case 500: // Feed override control
    case 510: // Spindle speed override control
    case 520: // Adaptive feed control
    case 530: // Feed stop control
    case 610: // Set current tool
    case 670: // Synchronized analog output
    case 680: // Immediate analog output

    default: implemented = false;
    }
    break;

  default: implemented = false;
  }

  // Update offsets
  updateOffsetParams();

  // Don't log really common codes
  if (implemented && (40 < code.number || code.type != 'G'))
    LOG_INFO(3, "Controller: " << code);

  return implemented;
}


void ControllerImpl::endBlock() {
  if (state.moveInAbsoluteCoords && currentMotionMode != 0 &&
      currentMotionMode != 10)
    LOG_WARNING(*Codes::find('G', 53) << " used without G0 or G1");
}
