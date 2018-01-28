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

#include "ControllerImpl.h"

#include <gcode/Codes.h>

#include <cbang/log/Logger.h>
#include <cbang/SStream.h>

#include <limits>

#include <string.h> // For memset()


using namespace std;
using namespace cb;
using namespace GCode;


ControllerImpl::ControllerImpl(MachineInterface &machine,
                               const ToolTable &tools) :
  tools(tools), synchronizing(false), currentMotionMode(10),
  plane(MachineInterface::XY),
  latheDiameterMode(true), cutterRadiusComp(false), toolLengthComp(false),
  pathMode(EXACT_PATH_MODE), returnMode(RETURN_TO_R),
  motionBlendingTolerance(0), naiveCamTolerance(0),
  incrementalDistanceMode(false), arcIncrementalDistanceMode(true),
  moveInAbsoluteCoords(false), feedMode(MachineInterface::UNITS_PER_MINUTE),
  spinMode(MachineInterface::REVOLUTIONS_PER_MINUTE), spindleDir(DIR_OFF),
  speed(0), maxSpindleSpeed(0) {

  this->machine.setParent(SmartPointer<MachineInterface>::Phony(&machine));

  memset(varValues, 0, sizeof(varValues));
}


double ControllerImpl::getVar(char c) const {return varValues[c - 'A'];}


const SmartPointer<Entity> &ControllerImpl::getVarExpr(char c) const {
  return varExprs[c - 'A'];
}


double ControllerImpl::getOffsetVar(char axis, bool absolute) const {
  LOG_INFO(6, "getOffsetVar(" << axis << ") var=" << getVar(axis) << " offset="
           << getAxisOffset(axis));

  return getVar(axis) +
    (absolute ? getAxisOffset(axis) : getAxisAbsolutePosition(axis));
}


string ControllerImpl::getVarGroupStr(const char *group) const {
  string s;

  for (int i = 0; group[i]; i++)
    s += SSTR(' ' << group[i] << getVar(group[i]));

  return s;
}


VarTypes::enum_t ControllerImpl::getVarType(char letter) {
  switch (letter) {
  case 'A': return VT_A;
  case 'B': return VT_B;
  case 'C': return VT_C;
  case 'D': return VT_D;
  case 'E': return VT_E;
  case 'F': return VT_F;
    // G
  case 'H': return VT_H;
  case 'I': return VT_I;
  case 'J': return VT_J;
  case 'K': return VT_K;
  case 'L': return VT_L;
    // M
    // N
    // O
  case 'P': return VT_P;
  case 'Q': return VT_Q;
  case 'R': return VT_R;
  case 'S': return VT_S;
  case 'T': return VT_T;
  case 'U': return VT_U;
  case 'V': return VT_V;
  case 'W': return VT_W;
  case 'X': return VT_X;
  case 'Y': return VT_Y;
  case 'Z': return VT_Z;
  default: THROWS("Invalid variable name " << letter);
  }
}


void ControllerImpl::setSpindleDir(dir_t dir) {
  spindleDir = dir;
  setSpeed(speed);
}


void ControllerImpl::setMistCoolant(bool enable) {
  machine.output(MachineEnum::MIST, enable);
}


void ControllerImpl::setFloodCoolant(bool enable) {
  machine.output(MachineEnum::FLOOD, enable);
}


void ControllerImpl::setPlane(MachineInterface::plane_t plane) {
  if (MachineInterface::VW < plane) THROW("Invalid plane");
  this->plane = plane;
}


const char *ControllerImpl::getPlaneAxes() const {
  switch (plane) {
  case MachineInterface::XY: return "XYZ";
  case MachineInterface::XZ: return "XZY";
  case MachineInterface::YZ: return "YZX";
  case MachineInterface::UV: return "UVW";
  case MachineInterface::UW: return "UWV";
  case MachineInterface::VW: return "VZU";
  default: THROWS("Invalid plane: " << plane);
  }
}


const char *ControllerImpl::getPlaneOffsets() const {
  switch (plane) {
  case MachineInterface::XY: return "IJ";
  case MachineInterface::XZ: return "IK";
  case MachineInterface::YZ: return "JK";
  case MachineInterface::UV: return "IJ";
  case MachineInterface::UW: return "IK";
  case MachineInterface::VW: return "JK";
  default: THROWS("Invalid plane: " << plane);
  }
}


double ControllerImpl::getAxisAbsolutePosition(char axis) const {
  return get(CURRENT_COORD_ADDR(Axes::toIndex(axis)));
}


void ControllerImpl::setAxisAbsolutePosition(char axis, double pos) {
  set(CURRENT_COORD_ADDR(Axes::toIndex(axis)), pos);
}


double ControllerImpl::getAxisOffset(char axis) const {
  double offset = 0;

  if (!moveInAbsoluteCoords) {
    unsigned axisIndex = Axes::toIndex(axis);

    // Coordinate system offset
    unsigned cs = get(CURRENT_COORD_SYSTEM);
    offset += get(COORD_SYSTEM_ADDR(cs, axisIndex));

    // Coordinate system rotation
    if (get(COORD_SYSTEM_ADDR(cs, COORD_SYSTEM_ROTATION_MEMBER))) {
      // TODO XY plane rotation
    }

    // Global offset (after CS offsets above)
    if (get(GLOBAL_OFFSETS_ENABLED))
      offset += get(GLOBAL_OFFSET_ADDR(axisIndex));
  }

  return offset;
}


double ControllerImpl::getAxisPosition(char axis) const {
  return getAxisAbsolutePosition(axis) - getAxisOffset(axis);
}


Axes ControllerImpl::getAbsolutePosition() const {
  Axes pos;

  for (const char *axes = Axes::AXES; *axes; axes++)
    pos.set(*axes, getAxisAbsolutePosition(*axes));

  LOG_INFO(5, "Controller: Current absolute position is " << pos);

  return pos;
}


void ControllerImpl::setAbsolutePosition(const Axes &axes) {
  LOG_INFO(5, "Controller: Set absolute position to " << axes);

  for (const char *var = Axes::AXES; *var; var++)
    setAxisAbsolutePosition(*var, axes.get(*var));
}


Axes ControllerImpl::getNextPosition(int vars, bool absolute) const {
  Axes pos;

  for (const char *axes = Axes::AXES; *axes; axes++)
    if (vars & getVarType(*axes))
      pos.set(*axes, getOffsetVar(*axes, absolute));
    else pos.set(*axes, getAxisAbsolutePosition(*axes));

  LOG_INFO(5, "Controller: Next position is " << pos);

  return pos;
}


void ControllerImpl::doMove(const Axes &pos, bool rapid) {
  machine.move(pos, rapid);
  setAbsolutePosition(pos);
}


void ControllerImpl::makeMove(int vars, bool rapid, bool absolute) {
  doMove(getNextPosition(vars, absolute), rapid);
}


void ControllerImpl::moveAxis(char axis, double value, bool rapid) {
  Axes pos = getAbsolutePosition();
  pos.set(axis, value);
  doMove(pos, rapid);
}


void ControllerImpl::arc(int vars, bool clockwise) {
  // TODO Affected by cutter radius compensation
  // TODO Make sure this is correct for planes XZ and YZ

  const char *axes = getPlaneAxes();
  if (plane == MachineInterface::XZ) clockwise = !clockwise;

  // Compute start and end points
  Axes current = getAbsolutePosition();
  Axes target = getNextPosition(vars, !incrementalDistanceMode);
  Vector2D start = Vector2D(current.get(axes[0]), current.get(axes[1]));
  Vector2D finish = Vector2D(target.get(axes[0]), target.get(axes[1]));
  Vector2D center;
  double radius;

  if (VT_R & vars) {
    radius = getVar('R');

    // If radius is less than half the distance then the arc is impossible
    double a = start.distance(finish) / 2;
    if (fabs(radius) < a - 0.00001) {
      LOG_WARNING("Impossible radius format arc, replacing with line segment, "
                  "radius=" << fabs(radius) << " distance/2=" << a);
      doMove(target, false);
      return;
    }

    // Compute arc center
    Vector2D m = (start + finish) / 2;
    Vector2D E =
      Vector2D(start.y() - finish.y(), finish.x() - start.x());
    double d = (finish - start).length() / 2;
    double l = sqrt(radius * radius - d * d);

    if (!clockwise) l = -l;
    if (0 < radius) l = -l;

    center = m + E.normalize() * l;

    static bool warned = false;
    if (!warned) LOG_WARNING("Radius format arcs are discouraged");
    warned = true;

  } else {
    // Get arc center
    const char *offsets = getPlaneOffsets();
    Vector2D offset;

    if (arcIncrementalDistanceMode) {
      offset =
        Vector2D(vars & getVarType(offsets[0]) ? getVar(offsets[0]) : 0,
                 vars & getVarType(offsets[1]) ? getVar(offsets[1]) : 0);
      center = start + offset;

    } else center = Vector2D(getVar(offsets[0]) + getAxisOffset(axes[0]),
                             getVar(offsets[1]) + getAxisOffset(axes[1]));

    // Check that the radius matches
    radius = start.distance(center);
    double radiusDiff = fabs(radius - finish.distance(center));
    if ((machine.isImperial() && 0.0002 < radiusDiff) ||
        (machine.isMetric() && 0.002 < radiusDiff)) {
      LOG_WARNING("Arc radiuses differ by " << radiusDiff);
      LOG_DEBUG(1, " center=" << center << " start=" << start << " finish="
                << finish << " offset=" << offset);
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
    angle = angle + M_PI * 2 * (getVar('P') - 1) * (angle < 0 ? -1 : 1);

  // Do arc
  double deltaZ = target.get(axes[2]) - current.get(axes[2]);
  Vector2D offset = center - start;
  machine.arc(Vector3D(offset.x(), offset.y(), deltaZ), -angle, plane);
  doMove(target, false);

  LOG_INFO(3, "Controller: Arc");
}


void ControllerImpl::straightProbe(int vars, bool towardWorkpiece,
                               bool signalError) {
  machine.seek(PROBE, towardWorkpiece, signalError);
  makeMove(vars, false, !incrementalDistanceMode);
  synchronizing = true; // Synchronize with found position

  LOG_INFO(3, "Controller: straight probe "
           << (towardWorkpiece ? "toward" : "away from") << " workpiece"
           << (signalError ? " with error signal" : ""));
}


void ControllerImpl::seek(int vars, bool active, bool error) {
  char targetAxis = 0;
  bool seekMin;

  for (const char *axes = Axes::AXES; *axes; axes++)
    if (vars & getVarType(*axes)) {
      if (targetAxis) THROW("Multiple axes in seek");
      targetAxis = *axes;
      seekMin = (0 < getVar(*axes)) ^ active;
    }

  port_t port;
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

  machine.seek(port, active, error);
  makeMove(vars, false, false);
  synchronizing = true; // Synchronize with found position
}


void ControllerImpl::drill(int vars, bool dwell, bool feedOut,
                           bool spindleStop) {
  unsigned xyVars = vars & (getPlaneXVarType() | getPlaneYVarType());
  unsigned zVar = getPlaneZVarType();
  double r = getVar('R');
  unsigned L = (vars & VT_L) ? (unsigned)getVar('L') : 1;

  double zClear = 0;
  switch (returnMode) {
  case RETURN_TO_R: zClear = r; break;
  case RETURN_TO_OLD_Z: {
    double oldZ = getAxisPosition(getPlaneZAxis());
    zClear = (r < oldZ) ? oldZ : r;
    break;
  }
  }

  for (unsigned i = 0; i < L; i++) {
    // Z is below R
    double z = getAxisPosition(getPlaneZAxis());
    if (!i && z < r) moveAxis(getPlaneZAxis(), r, true);

    // Traverse to XY
    makeMove(xyVars, true, !incrementalDistanceMode);

    // Traverse to Z if above
    z = getAxisPosition(getPlaneZAxis());
    if (z != r) moveAxis(getPlaneZAxis(), r, true);

    // Drill
    makeMove(zVar, false, !incrementalDistanceMode);

    // Dwell
    if (dwell) this->dwell(getVar('P'));

    // Stop Spindle
    dir_t spindleDir = this->spindleDir;
    if (spindleStop) setSpindleDir(DIR_OFF);

    // Retract
    moveAxis(getPlaneZAxis(), zClear, !feedOut);

    // Restart Spindle
    if (spindleStop) setSpindleDir(spindleDir);
  }
}


void ControllerImpl::dwell(double seconds) {machine.dwell(seconds);}


void ControllerImpl::setCoordSystemRotation(unsigned cs, double rotation) {
  set(COORD_SYSTEM_ADDR(cs, COORD_SYSTEM_ROTATION_MEMBER), rotation);
}


void ControllerImpl::setCoordSystemOffset(unsigned cs, char axis,
                                          bool relative) {
  double offset = getVar(axis);
  if (relative) offset = getAxisPosition(axis) - offset;

  set(COORD_SYSTEM_ADDR(cs, Axes::toIndex(axis)), offset);
}


void ControllerImpl::setCoordSystem(int vars, bool relative) {
  unsigned cs = getVar('P');
  if (9 < cs) THROWS("Invalid coordinate system number " << cs);
  if (!cs) cs = get(CURRENT_COORD_SYSTEM);

  if (vars & VarTypes::VT_R) setCoordSystemRotation(cs, getVar('R'));

  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis))
      setCoordSystemOffset(cs, *axis, relative);
}


void ControllerImpl::setToolTable(int vars, bool relative) {
  Tool &tool = tools.get(getVar('P'));

  for (const char *v = Tool::VARS; *v; v++)
    if (vars & getVarType(*v)) {
      double value = getVar(*v);
      if (relative && (getVarType(*v) & VT_AXIS))
        value -= getAxisOffset(*v); // TODO What about R, I, J, Q offsets?
      tool.set(*v, value);
    }

  LOG_INFO(3, "Controller: Set Tool Table" << getVarGroupStr("PRXYZABCUVWIJQ"));
}


void ControllerImpl::toolChange() {
  int tool = get("_selected_tool");
  if (tool < 0) THROW("No tool selected");
  machine.changeTool(tool);
  LOG_INFO(3, "Controller: Tool change " << get(TOOL_NUMBER));
}


void ControllerImpl::loadToolOffsets(unsigned number) {
  const Tool &tool = getTool(number);

  for (const char *axis = Axes::AXES; *axis; axis++)
    set(TOOL_OFFSET_ADDR(Axes::toIndex(*axis)), tool.get(*axis));
}


void ControllerImpl::loadToolVarOffsets(int vars) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis))
      // TODO do these need to be offset?
      set(TOOL_OFFSET_ADDR(Axes::toIndex(*axis)), getVar(*axis));
}


void ControllerImpl::storePredefined(bool first) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    set(PREDEFINED_ADDR(first, Axes::toIndex(*axis)),
        getAxisAbsolutePosition(*axis));
}


void ControllerImpl::loadPredefined(bool first, int vars) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (vars & getVarType(*axis))
      setAxisAbsolutePosition
        (*axis, get(PREDEFINED_ADDR(first, Axes::toIndex(*axis))));
}


void ControllerImpl::setGlobalOffsets(int vars) {
  // TODO Test G52/G92
  set(GLOBAL_OFFSETS_ENABLED, 1);

  // Make the current point have the coordinates specified, without motion
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (getVarType(*axis) & vars) {
      gcode_address_t addr = GLOBAL_OFFSET_ADDR(Axes::toIndex(*axis));
      double offset = getAxisPosition(*axis) - getVar(*axis);

      // Update global offset
      set(addr, get(addr) + offset);
    }
}


void ControllerImpl::resetGlobalOffsets(bool clear) {
  set(GLOBAL_OFFSETS_ENABLED, 0);

  if (clear)
    for (unsigned axis = 0; axis < 9; axis++)
      set(GLOBAL_OFFSET_ADDR(axis), 0);
}


void ControllerImpl::restoreGlobalOffsets() {set(GLOBAL_OFFSETS_ENABLED, 1);}


void ControllerImpl::setAxisHomed(int vars, bool homed) {
  for (const char *axis = Axes::AXES; *axis; axis++)
    if (getVarType(*axis) & vars) {
      set(SSTR("_" << (char)tolower(*axis) << "_homed"), homed);
      set(SSTR("_" << (char)tolower(*axis) << "_home"),
          homed ? getVar(*axis) : numeric_limits<double>::quiet_NaN());
    }
}


void ControllerImpl::setCutterRadiusComp(int vars, bool left, bool dynamic) {
  if (dynamic) {
    set(TOOL_DIAMETER, (left ? 1 : -1) * getVar('D'));
    if (vars & VT_L) set(TOOL_ORIENTATION, getVar('L'));
    else set(TOOL_ORIENTATION, 0);

  } else {
    set(TOOL_ORIENTATION, 0);

    if (!(vars & VT_D))
      set(TOOL_DIAMETER, getTool(getCurrentTool()).getRadius() * 2);
    else if (!getVar('D')) set(TOOL_DIAMETER, 0);
    else set(TOOL_DIAMETER, getTool(getVar('D')).getRadius() * 2);
  }

  cutterRadiusComp = true;
}


void ControllerImpl::end() {
  // See http://linuxcnc.org/docs/html/gcode/m-code.html#mcode:m2-m30
  //
  // These commands have the following effects:
  //
  //   Change from Auto mode to MDI mode.
  // N/A

  //   Origin offsets are set to the default (like G54).
  set(CURRENT_COORD_SYSTEM, 1);

  //   Selected plane is set to XY plane (like G17).
  setPlane(MachineInterface::XY);

  //   Distance mode is set to absolute mode (like G90).
  incrementalDistanceMode = false;

  //   Feed rate mode is set to units per minute (like G94).
  feedMode = MachineInterface::UNITS_PER_MINUTE;

  //   Feed and speed overrides are set to ON (like M48).
  // TODO

  //   Cutter compensation is turned off (like G40).
  cutterRadiusComp = false;

  //   The spindle is stopped (like M5).
  setSpindleDir(DIR_OFF);

  //   The current motion mode is set to feed (like G1).
  setCurrentMotionMode(10);

  //   Coolant is turned off (like M9).
  setMistCoolant(false);
  setFloodCoolant(false);

  throw EndProgram();
}


double ControllerImpl::get(gcode_address_t addr) const {
  return machine.get(addr);
}


void ControllerImpl::set(gcode_address_t addr, double value) {
  machine.set(addr, value);
}


bool ControllerImpl::has(const string &name) const {return machine.has(name);}
double ControllerImpl::get(const string &name) const {return machine.get(name);}


void ControllerImpl::set(const string &name, double value) {
  machine.set(name, value);
}


void ControllerImpl::setVar(char c, double value) {varValues[c - 'A'] = value;}


void ControllerImpl::setVarExpr(char c, const SmartPointer<Entity> &entity) {
  varExprs[c - 'A'] = entity;
}


void ControllerImpl::setCurrentMotionMode(unsigned mode) {
  currentMotionMode = mode;
}


void ControllerImpl::synchronize(const Axes &position) {
  if (!synchronizing) THROW("Not synchronizing");
  setAbsolutePosition(position);
  // TODO Also set probed position PROBE_RESULT_X-W and PROBE_SUCCESS
  synchronizing = false;
}


void ControllerImpl::setLocation(const LocationRange &location) {
  machine.setLocation(location);
}


void ControllerImpl::setFeed(double feed) {machine.setFeed(feed, feedMode);}


void ControllerImpl::setSpeed(double speed) {
  this->speed = speed;

  double mspeed;

  switch (spindleDir) {
  case DIR_OFF: mspeed = 0; break;
  case DIR_CLOCKWISE: mspeed = speed; break;
  case DIR_COUNTERCLOCKWISE: mspeed = -speed; break;
  default: THROW("Invalid spindle direction");
  }

  machine.setSpeed(mspeed, spinMode, maxSpindleSpeed);
}


void ControllerImpl::setTool(unsigned tool) {
  machine.set("_selected_tool", tool);
}


void ControllerImpl::newBlock() {
  if (synchronizing) {
    LOG_WARNING("New block started without position of previous seek move");
    synchronizing = false;
  }
  moveInAbsoluteCoords = false;
}


bool ControllerImpl::execute(const Code &code, int vars) {
  bool implemented = true;
  bool verbose = true;

  switch (code.type) {
  case 'G':
    switch (code.number) {
    case 0:
      makeMove(vars, true, !incrementalDistanceMode);
      verbose = false;
      break;

    case 10:
      makeMove(vars, false, !incrementalDistanceMode);
      verbose = false;
      break;

    case 20: arc(vars, true);    verbose = false; break;
    case 30: arc(vars, false);   verbose = false; break;
    case 40: dwell(getVar('P')); verbose = false; break;

    case 51: implemented = false; // TODO Quadratic B-spline
    case 52: implemented = false; // TODO NURBS Block Start
    case 53: implemented = false; // TODO NURBS Block End

    case 70: latheDiameterMode = true;  break;
    case 80: latheDiameterMode = false; break; // Radius mode

    case 100:
      switch ((unsigned)getVar('L')) {
      case 1:  setToolTable(vars,   false); break;
      case 2:  setCoordSystem(vars, false); break;
      case 10: setToolTable(vars,   true);  break;
      case 20: setCoordSystem(vars, true);  break;
      }
      break;

    case 170: setPlane(MachineInterface::XY); break;
    case 171: setPlane(MachineInterface::UV); break;
    case 180: setPlane(MachineInterface::XZ); break;
    case 181: setPlane(MachineInterface::UW); break;
    case 190: setPlane(MachineInterface::YZ); break;
    case 191: setPlane(MachineInterface::VW); break;

    case 200: machine.setImperial(); break;
    case 210: machine.setMetric();   break;

    case 280:
      if (vars & VT_AXIS) {
        makeMove(vars, true, true);
        loadPredefined(true, vars);

      } else loadPredefined(true, VT_AXIS);

      makeMove(0, true, true);
      break;

    case 281: storePredefined(true); break;

    case 282: setAxisHomed(vars, false); break;
    case 283: setAxisHomed(vars, true);  break;

    case 300:
      if (vars & VT_AXIS) {
        makeMove(vars, true, true);
        loadPredefined(false, vars);

      } else loadPredefined(false, VT_AXIS);

      makeMove(0, true, true);
      break;

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

    case 400: cutterRadiusComp = false; break;
    case 410: setCutterRadiusComp(vars, true, false);  break;
    case 411: setCutterRadiusComp(vars, true, true);   break;
    case 420: setCutterRadiusComp(vars, false, false); break;
    case 421: setCutterRadiusComp(vars, false, true);  break;

    case 430:
      toolLengthComp = true;
      loadToolOffsets((vars & VT_H) ? getVar('H') : getCurrentTool());
      break;
    case 431:
      toolLengthComp = true;
      loadToolVarOffsets(vars);
      break;
    case 490: toolLengthComp = false; break;

    case 520: setGlobalOffsets(vars); break; // Same as G92
    case 530: moveInAbsoluteCoords = true;  break;
    case 540: set(CURRENT_COORD_SYSTEM, 1); break;
    case 550: set(CURRENT_COORD_SYSTEM, 2); break;
    case 560: set(CURRENT_COORD_SYSTEM, 3); break;
    case 570: set(CURRENT_COORD_SYSTEM, 4); break;
    case 580: set(CURRENT_COORD_SYSTEM, 5); break;
    case 590: set(CURRENT_COORD_SYSTEM, 6); break;
    case 591: set(CURRENT_COORD_SYSTEM, 7); break;
    case 592: set(CURRENT_COORD_SYSTEM, 8); break;
    case 593: set(CURRENT_COORD_SYSTEM, 9); break;

    case 610: pathMode = EXACT_PATH_MODE; break;
    case 611: pathMode = EXACT_STOP_MODE; break;
    case 640:
      pathMode = CONTINUOUS_MODE;
      if (vars & VT_P) motionBlendingTolerance = getVar('P');
      else motionBlendingTolerance = 0;
      if (vars & VT_Q) naiveCamTolerance = getVar('Q');
      else naiveCamTolerance = 0;
      break;

    case 730: implemented = false; break; // Drill cycle w/ chip breaking
    case 760: implemented = false; break; // Thread cycle

    case 800:                                   break; // Cancel canned cycle
    case 810: drill(vars, false, false, false); break; // Drill cycle
    case 820: drill(vars, true, false, false);  break; // Drill cycle w/ dwell
    case 830: implemented = false;              break; // Peck drill
    case 840: implemented = false;              break; // Right-hand tap
    case 850: drill(vars, false, true, false);  break; // No dwell, feed out
    case 860: drill(vars, false, false, true);  break; // Spin stop rapid out
    case 870: implemented = false;              break; // Back boring
    case 880: implemented = false;              break; // Spin stop manual out
    case 890: drill(vars, true, true, false);   break; // Dwell, feed out

    case 900: incrementalDistanceMode = false;    break;
    case 901: arcIncrementalDistanceMode = false; break;
    case 910: incrementalDistanceMode = true;     break;
    case 911: arcIncrementalDistanceMode = true;  break;

    case 920: setGlobalOffsets(vars);    break;
    case 921: resetGlobalOffsets(true);  break;
    case 922: resetGlobalOffsets(false); break;
    case 923: restoreGlobalOffsets();    break;

    case 930: feedMode = MachineInterface::INVERSE_TIME;      break;
    case 940: feedMode = MachineInterface::UNITS_PER_MINUTE;     break;
    case 950: feedMode = MachineInterface::UNITS_PER_REVOLUTION; break;

      // NOTE: The spindle modes must be accompanied by a speed
    case 960:
      spinMode = MachineInterface::CONSTANT_SURFACE_SPEED;
      maxSpindleSpeed = getVar('D');
      break;
    case 970: spinMode = MachineInterface::REVOLUTIONS_PER_MINUTE; break;

    case 980: returnMode = RETURN_TO_R;     break;
    case 990: returnMode = RETURN_TO_OLD_Z; break;

    default: implemented = false;
    }
    break;

  case 'M':
    switch (code.number) {
    case 0:                                       break; // TODO Pause
    case 10:                                      break; // TODO Optional pause
    case 20: end();                               break; // End program
    case 30: setSpindleDir(DIR_CLOCKWISE);        break;
    case 40: setSpindleDir(DIR_COUNTERCLOCKWISE); break;
    case 50: setSpindleDir(DIR_OFF);              break;
    case 60: toolChange();                        break; // Manual Tool Change
    case 70: setMistCoolant(true);                break;
    case 80: setFloodCoolant(true);               break;
    case 90:
      setMistCoolant(false);
      setFloodCoolant(false);
      break;

      // TODO the following M-Codes
    case 190: implemented = false; break; // Orient spindle
    case 300: end(); break; // TODO Exchange pallet shuttles and end program
    case 480: case 490: // Speed and feed override control
    case 500: // Feed override control
    case 510: // Spindle speed override control
    case 520: // Adaptive feed control
    case 530: // Feed stop control
    case 600: // Pallet change pause
    case 610: // Set current tool
    case 620: case 630: case 640: case 650: // Digital output
    case 660: // Wait on input
    case 670: // Synchronized analog output
    case 680: // Imediate analog output
    case 700: // Save modal state
    case 710: // Invalidate stored state
    case 720: // Restore modal state
    case 730: // Save and Auto-restore modal state

    default: implemented = false;
    }
    break;

  default: implemented = false;
  }

  if (implemented && verbose) LOG_INFO(3, "Controller: " << code);
  return implemented;
}
