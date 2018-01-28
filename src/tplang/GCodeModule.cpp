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

#include "GCodeModule.h"
#include "TPLContext.h"

#include <gcode/ToolTable.h>
#include <gcode/ControllerImpl.h>
#include <gcode/interp/Interpreter.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/geom/Rectangle.h>

#include <math.h>

using namespace tplang;
using namespace GCode;
using namespace std;
using namespace cb;


GCodeModule::GCodeModule(TPLContext &ctx) :
  js::NativeModule("gcode"), ctx(ctx), unitAdapter(0) {}


void GCodeModule::define(js::Sink &exports) {
#define XYZ "x, y, z"
#define ABC "a, b, c"
#define UVW "u, v, w"
#define IJK "i, j, k"
#define AXES XYZ ", " ABC ", " UVW

  exports.insert("gcode(path)", this, &GCodeModule::gcodeCB);
  exports.insert("rapid(" AXES ", incremental=false)", this,
                 &GCodeModule::rapidCB);
  exports.insert("irapid(" AXES ", incremental=true)", this,
                 &GCodeModule::rapidCB);
  exports.insert("cut(" AXES ", incremental=false)", this, &GCodeModule::cutCB);
  exports.insert("icut(" AXES ", incremental=true)", this, &GCodeModule::cutCB);
  exports.insert("arc(x=0, y=0, z=0, angle, plane, incremental=true)", this,
                 &GCodeModule::arcCB);
  exports.insert("probe(" AXES ", port=0, active=true, error=true)", this,
                 &GCodeModule::probeCB);
  exports.insert("dwell(seconds)", this, &GCodeModule::dwellCB);
  exports.insert("feed(rate, mode)", this, &GCodeModule::feedCB);
  exports.insert("speed(rate, surface, max)", this, &GCodeModule::speedCB);
  exports.insert("tool(number)", this, &GCodeModule::toolCB);
  exports.insert("units(type)", this, &GCodeModule::unitsCB);
  exports.insert("pause(optional=false)", this, &GCodeModule::pauseCB);
  exports.insert("position()", this, &GCodeModule::positionCB);
  exports.insert("comment(...)", this, &GCodeModule::commentCB);
  exports.insert("workpiece()", this, &GCodeModule::workpieceCB);

  exports.insert("FEED_INVERSE_TIME", INVERSE_TIME);
  exports.insert("FEED_UNITS_PER_MIN", UNITS_PER_MINUTE);
  exports.insert("FEED_UNITS_PER_REV", UNITS_PER_REVOLUTION);

  exports.insert("IMPERIAL", Units::IMPERIAL);
  exports.insert("METRIC", Units::METRIC);

  exports.insert("XY", MachineEnum::XY);
  exports.insert("UV", MachineEnum::UV);
  exports.insert("XZ", MachineEnum::XZ);
  exports.insert("UW", MachineEnum::UW);
  exports.insert("YZ", MachineEnum::YZ);
  exports.insert("VW", MachineEnum::VW);

  exports.insert("CYLINDRICAL", ToolShape::TS_CYLINDRICAL);
  exports.insert("CONICAL", ToolShape::TS_CONICAL);
  exports.insert("BALLNOSE", ToolShape::TS_BALLNOSE);
  exports.insert("SPHEROID", ToolShape::TS_SPHEROID);
  exports.insert("SNUBNOSE", ToolShape::TS_SNUBNOSE);

  exports.insert("X_MIN", MachineEnum::X_MIN);
  exports.insert("X_MAX", MachineEnum::X_MAX);
  exports.insert("Y_MIN", MachineEnum::Y_MIN);
  exports.insert("Y_MAX", MachineEnum::Y_MAX);
  exports.insert("Z_MIN", MachineEnum::Z_MIN);
  exports.insert("Z_MAX", MachineEnum::Z_MAX);
  exports.insert("A_MIN", MachineEnum::A_MIN);
  exports.insert("A_MAX", MachineEnum::A_MAX);
  exports.insert("B_MIN", MachineEnum::B_MIN);
  exports.insert("B_MAX", MachineEnum::B_MAX);
  exports.insert("C_MIN", MachineEnum::C_MIN);
  exports.insert("C_MAX", MachineEnum::C_MAX);
  exports.insert("U_MIN", MachineEnum::U_MIN);
  exports.insert("U_MAX", MachineEnum::U_MAX);
  exports.insert("V_MIN", MachineEnum::V_MIN);
  exports.insert("V_MAX", MachineEnum::V_MAX);
  exports.insert("W_MIN", MachineEnum::W_MIN);
  exports.insert("W_MAX", MachineEnum::W_MAX);
  exports.insert("PROBE", MachineEnum::PROBE);
  exports.insert("FLOOD", MachineEnum::FLOOD);
  exports.insert("MIST",  MachineEnum::MIST);

#undef XYZ
#undef ABC
#undef UVW
#undef IJK
#undef AXES
}


MachineUnitAdapter &GCodeModule::getUnitAdapter() {
  if (!unitAdapter) unitAdapter = &ctx.find<MachineUnitAdapter>();
  return *unitAdapter;
}


void GCodeModule::gcodeCB(const js::Value &args, js::Sink &sink) {
  string path =
    SystemUtilities::absolute(ctx.getCurrentPath(), args.getString("path"));

  ctx.pushPath(path);
  SmartFunctor<TPLContext> popPath(&ctx, &TPLContext::popPath);

  Options options;
  ControllerImpl controller(ctx.machine);
  Interpreter(controller).read(path);
}


void GCodeModule::rapidCB(const js::Value &args, js::Sink &sink) {
  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes, args.getBoolean("incremental"));
  ctx.machine.move(axes, true);
}


void GCodeModule::cutCB(const js::Value &args, js::Sink &sink) {
  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes, args.getBoolean("incremental"));
  ctx.machine.move(axes);
}


void GCodeModule::arcCB(const js::Value &args, js::Sink &sink) {
  // TODO Handle 'incremental=false'

  Vector3D
    offset(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"));
  double angle = args.getNumber("angle");
  plane_t plane = args.has("plane") ? (plane_t)args.getInteger("plane") : XY;

  ctx.machine.arc(offset, angle, plane);
}


void GCodeModule::probeCB(const js::Value &args, js::Sink &sink) {
  ctx.machine.seek((MachineEnum::port_t)args.getInteger("port"),
                   args.getBoolean("active"), args.getBoolean("error"));

  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes);
  ctx.machine.move(axes);
}


void GCodeModule::dwellCB(const js::Value &args, js::Sink &sink) {
  ctx.machine.dwell(args.getNumber("seconds"));
}


void GCodeModule::feedCB(const js::Value &args, js::Sink &sink) {
  // Return feed info if no arguments were given
  if (!args.has("rate")) {
    feed_mode_t mode;

    sink.beginList();
    sink.append(ctx.machine.getFeed(&mode));
    sink.append(mode);
    sink.endList();

    return;
  }

  // Otherwise set feed
  feed_mode_t mode = UNITS_PER_MINUTE;
  if (args.has("mode")) {
    mode = (feed_mode_t)args.getInteger("mode");
    switch (mode) {
    case INVERSE_TIME: case UNITS_PER_MINUTE: case UNITS_PER_REVOLUTION: break;
    default: THROW("Feed mode must be FEED_INVERSE_TIME, FEED_UNITS_PER_MIN or "
                   "FEED_UNITS_PER_REV");
    }
  }

  ctx.machine.setFeed(args.getNumber("rate"), mode);

  sink.write(ctx.machine.getFeed());
}


void GCodeModule::speedCB(const js::Value &args, js::Sink &sink) {
  // Return spindle info if no arguments were given
  if (!args.has("rate")) {
    spin_mode_t mode;
    double max;

    sink.beginList();
    sink.append(ctx.machine.getSpeed(&mode, &max));
    sink.append(mode);
    sink.append(max);
    sink.endList();

    return;
  }

  // Otherwise set spindle
  spin_mode_t mode = REVOLUTIONS_PER_MINUTE;
  double max = 0;

  if (args.has("surface")) {
    mode = CONSTANT_SURFACE_SPEED;
    if (args.has("max")) max = args.getNumber("max");
  }

  ctx.machine.setSpeed(args.getNumber("rate"), mode, max);

  sink.write(ctx.machine.getSpeed());
}


void GCodeModule::toolCB(const js::Value &args, js::Sink &sink) {
  int number;

  if (args.has("number")) {
    number = args.getInteger("number");
    ctx.machine.changeTool(number);

  } else number = ctx.machine.get(TOOL_NUMBER);

  if (number < 0) return;

  if (!ctx.sim->hasDict("tools")) return;

  ToolTable tools;
  tools.read(ctx.sim->getDict("tools"));

  if (!tools.has(number)) return;

  Tool tool = tools.get(number);

  double scale = getUnitAdapter().getUnits() == Units::METRIC ? 1 : 25.4;

  sink.beginDict();
  sink.insert("number", number);
  sink.insert("shape", (uint32_t)tool.getShape());
  if (tool.getShape() == ToolShape::TS_CONICAL)
    sink.insert("angle", tool.getAngle());
  sink.insert("length", tool.getLength() / scale);
  sink.insert("diameter", tool.getDiameter() / scale);
  if (tool.getShape() == ToolShape::TS_SNUBNOSE)
    sink.insert("snub_diameter", tool.getSnubDiameter() / scale);
  sink.endDict();
}


void GCodeModule::unitsCB(const js::Value &args, js::Sink &sink) {
  Units units = getUnitAdapter().getUnits();

  if (args.has("type")) {
    switch (args.getInteger("type")) {
    case Units::IMPERIAL:
      getUnitAdapter().setUnits(Units::IMPERIAL);
      break;
    case Units::METRIC:
      getUnitAdapter().setUnits(Units::METRIC);
      break;
    default: THROWS("Units type must be one of IMPERIAL or METRIC");
    }

  } else sink.write(units);
}


void GCodeModule::pauseCB(const js::Value &args, js::Sink &sink) {
  ctx.machine.pause(args.getBoolean("optional"));
}


void GCodeModule::positionCB(const js::Value &args, js::Sink &sink) {
  Axes axes = ctx.machine.getPosition();

  sink.beginDict();

  for (unsigned i = 0; Axes::AXES[i]; i++)
    sink.insert(string(1, tolower(Axes::AXES[i])), axes.getIndex(i));

  sink.endDict();
}


void GCodeModule::commentCB(const js::Value &args, js::Sink &sink) {
  for (unsigned i = 0; i < args.length(); i++)
    ctx.machine.comment(args.getString(i)); // TODO Call JSON.stringify()
}


void GCodeModule::workpieceCB(const js::Value &args, js::Sink &sink) {
  Rectangle3D workpiece;
  workpiece.read(ctx.sim->getDict("workpiece"));

  if (getUnitAdapter().getUnits() == Units::IMPERIAL)
    workpiece = workpiece / 25.4;

  Vector3D dims = workpiece.getDimensions();
  const Vector3D &offset = workpiece.getMin();

  sink.beginDict();

  sink.insertDict("dims");
  sink.insert("x", dims.x());
  sink.insert("y", dims.y());
  sink.insert("z", dims.z());
  sink.endDict();

  sink.insertDict("offset");
  sink.insert("x", offset.x());
  sink.insert("y", offset.y());
  sink.insert("z", offset.z());
  sink.endDict();

  sink.endDict();
}


void GCodeModule::parseAxes(const js::Value &args, Axes &axes,
                            bool incremental) {
  for (const char *axis = "xyzabcuvw"; *axis; axis++) {
    string name = string(1, *axis);
    if (!args.has(name)) continue;

    double value = args.getNumber(name) + (incremental ? axes.get(*axis) : 0);
    if (!Math::isfinite(value)) THROWS(*axis << " position is invalid");

    axes.set(*axis, value);
  }
}
