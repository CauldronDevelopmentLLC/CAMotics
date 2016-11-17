/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#include "GCodeModule.h"
#include "TPLContext.h"

#include <camotics/sim/ToolTable.h>
#include <camotics/sim/Controller.h>
#include <camotics/gcode/Interpreter.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/util/SmartFunctor.h>

#include <math.h>

using namespace tplang;
using namespace CAMotics;
using namespace std;
using namespace cb;


GCodeModule::GCodeModule(TPLContext &ctx) : ctx(ctx), unitAdapter(0) {}


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
  exports.insert("probe(" AXES ", toward=true, error=true, index=0, port=-1, "
                 "invert=false)", this, &GCodeModule::probeCB);
  exports.insert("dwell(seconds)", this, &GCodeModule::dwellCB);
  exports.insert("feed(rate, mode)", this, &GCodeModule::feedCB);
  exports.insert("speed(rate, surface, max)", this, &GCodeModule::speedCB);
  exports.insert("tool(number)", this, &GCodeModule::toolCB);
  exports.insert("units(type)", this, &GCodeModule::unitsCB);
  exports.insert("pause(optional=false)", this, &GCodeModule::pauseCB);
  exports.insert("tool_set(number, length, diameter, units, shape, snub=0)",
                 this, &GCodeModule::toolSetCB);
  exports.insert("position()", this, &GCodeModule::positionCB);

  exports.insert("FEED_INVERSE_TIME", INVERSE_TIME);
  exports.insert("FEED_UNITS_PER_MIN", MM_PER_MINUTE);
  exports.insert("FEED_UNITS_PER_REV", MM_PER_REVOLUTION);

  exports.insert("IMPERIAL", CAMotics::Units::IMPERIAL);
  exports.insert("METRIC", CAMotics::Units::METRIC);

  exports.insert("XY", XY);
  exports.insert("XZ", XZ);
  exports.insert("YZ", YZ);
  exports.insert("YV", YV);
  exports.insert("UV", UV);
  exports.insert("VW", VW);

  exports.insert("CYLINDRICAL", CAMotics::ToolShape::TS_CYLINDRICAL);
  exports.insert("CONICAL", CAMotics::ToolShape::TS_CONICAL);
  exports.insert("BALLNOSE", CAMotics::ToolShape::TS_BALLNOSE);
  exports.insert("SPHEROID", CAMotics::ToolShape::TS_SPHEROID);
  exports.insert("SNUBNOSE", CAMotics::ToolShape::TS_SNUBNOSE);

#undef XYZ
#undef ABC
#undef UVW
#undef IJK
#undef AXES
}


CAMotics::MachineUnitAdapter &GCodeModule::getUnitAdapter() {
  if (!unitAdapter) unitAdapter = &ctx.find<CAMotics::MachineUnitAdapter>();
  return *unitAdapter;
}


void GCodeModule::gcodeCB(const js::Value &args, js::Sink &sink) {
  string path =
    SystemUtilities::absolute(ctx.getCurrentPath(), args.getString("path"));

  ctx.pushPath(path);
  SmartFunctor<TPLContext> popPath(&ctx, &TPLContext::popPath);

  Options options;
  CAMotics::Controller controller(ctx.machine);
  CAMotics::Interpreter(controller).read(path);
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
  bool toward = args.getBoolean("toward");
  bool error = args.getBoolean("error");
  uint32_t index = args.getInteger("index");
  int port = args.getInteger("port");
  bool invert = args.getBoolean("invert");

  if (port == -1) port = ctx.machine.findPort(PROBE, index);
  if (port != -1)
    ctx.machine.input(port, toward ^ invert ? STOP_WHEN_HIGH : STOP_WHEN_LOW,
                      error);

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
  feed_mode_t mode = MM_PER_MINUTE;
  if (args.has("mode")) {
    mode = (feed_mode_t)args.getInteger("mode");
    switch (mode) {
    case INVERSE_TIME: case MM_PER_MINUTE: case MM_PER_REVOLUTION: break;
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
    ctx.sim.tools.get(number); // Make sure it exists
    ctx.machine.setTool(number);

  } else number = ctx.machine.getTool();

  if (number < 0) return;

  const Tool &tool = ctx.sim.tools.get(number);

  double scale =
    getUnitAdapter().getUnits() == CAMotics::Units::METRIC ? 1 : 25.4;

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
  CAMotics::Units units = getUnitAdapter().getUnits();

  if (args.has("type")) {
    switch (args.getInteger("type")) {
    case CAMotics::Units::IMPERIAL:
      getUnitAdapter().setUnits(CAMotics::Units::IMPERIAL);
      break;
    case CAMotics::Units::METRIC:
      getUnitAdapter().setUnits(CAMotics::Units::METRIC);
      break;
    default: THROWS("Units type must be one of IMPERIAL or METRIC");
    }

  } else sink.write(units);
}


void GCodeModule::pauseCB(const js::Value &args, js::Sink &sink) {
  ctx.machine.pause(args.getBoolean("optional"));
}


void GCodeModule::toolSetCB(const js::Value &args, js::Sink &sink) {
  CAMotics::Tool &tool = ctx.sim.tools.get(args.getInteger("number"));

  uint32_t units;
  double scale = 1;
  if (args.has("units")) units = args.getInteger("units");
  else units = getUnitAdapter().getUnits();
  if (units == CAMotics::Units::METRIC)
    tool.setUnits(CAMotics::ToolUnits::UNITS_MM);
  else {
    tool.setUnits(CAMotics::ToolUnits::UNITS_INCH);
    scale = 25.4;
  }

  if (args.has("shape"))
    tool.setShape((CAMotics::ToolShape::enum_t)args.getInteger("shape"));

  tool.setLength(scale * args.getNumber("length"));
  tool.setDiameter(scale * args.getNumber("diameter"));
  tool.setSnubDiameter(scale * args.getNumber("snub"));
}


void GCodeModule::positionCB(const js::Value &args, js::Sink &sink) {
  Axes axes = ctx.machine.getPosition();

  sink.beginDict();

  for (unsigned i = 0; Axes::AXES[i]; i++)
    sink.insert(string(1, tolower(Axes::AXES[i])), axes.getIndex(i));

  sink.endDict();
}


void GCodeModule::parseAxes(const js::Value &args, Axes &axes,
                            bool incremental) {
  for (const char *axis ="xyzabcuvw"; *axis; axis++) {
    string name = string(1, *axis);
    if (args.has(name)) axes.set(*axis, args.getNumber(name) +
                                 (incremental ? axes.get(*axis) : 0));
  }
}
