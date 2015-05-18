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

#include "GCodeModule.h"

#include <openscam/sim/ToolTable.h>
#include <openscam/sim/Controller.h>
#include <openscam/gcode/Interpreter.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/util/SmartFunctor.h>

#include <math.h>

using namespace tplang;
using namespace std;
using namespace cb;


void GCodeModule::define(js::ObjectTemplate &exports) {
#define XYZ "x, y, z"
#define ABC "a, b, c"
#define UVW "u, v, w"
#define IJK "i, j, k"
#define AXES XYZ ", " ABC ", " UVW

  exports.set("gcode(path)", this, &GCodeModule::gcodeCB);
  exports.set("rapid(" AXES ", incremental=false)", this,
              &GCodeModule::rapidCB);
  exports.set("irapid(" AXES ", incremental=true)", this,
              &GCodeModule::rapidCB);
  exports.set("cut(" AXES ", incremental=false)", this, &GCodeModule::cutCB);
  exports.set("icut(" AXES ", incremental=true)", this, &GCodeModule::cutCB);
  exports.set("arc(x=0, y=0, z=0, angle, plane, incremental=true)", this,
          &GCodeModule::arcCB);
  exports.set("probe(" AXES ", toward=true, error=true, index=0, port=-1, "
          "invert=false)", this, &GCodeModule::probeCB);
  exports.set("dwell(seconds)", this, &GCodeModule::dwellCB);
  exports.set("feed(rate, mode)", this, &GCodeModule::feedCB);
  exports.set("speed(rate, surface, max)", this, &GCodeModule::speedCB);
  exports.set("tool(number)", this, &GCodeModule::toolCB);
  exports.set("units(type)", this, &GCodeModule::unitsCB);
  exports.set("pause(optional=false)", this, &GCodeModule::pauseCB);
  exports.set("tool_set(number, length, diameter, units, shape, snub=0, "
           "front_angle=0, back_angle=0, orientation=0)", this,
           &GCodeModule::toolSetCB);
  exports.set("position()", this, &GCodeModule::positionCB);

  exports.set("FEED_INVERSE_TIME", INVERSE_TIME);
  exports.set("FEED_UNITS_PER_MIN", MM_PER_MINUTE);
  exports.set("FEED_UNITS_PER_REV", MM_PER_REVOLUTION);

  exports.set("IMPERIAL", MachineUnitAdapter::IMPERIAL);
  exports.set("METRIC", MachineUnitAdapter::METRIC);

  exports.set("XY", XY);
  exports.set("XZ", XZ);
  exports.set("YZ", YZ);
  exports.set("YV", YV);
  exports.set("UV", UV);
  exports.set("VW", VW);

  exports.set("CYLINDRICAL", OpenSCAM::ToolShape::TS_CYLINDRICAL);
  exports.set("CONICAL", OpenSCAM::ToolShape::TS_CONICAL);
  exports.set("BALLNOSE", OpenSCAM::ToolShape::TS_BALLNOSE);
  exports.set("SPHEROID", OpenSCAM::ToolShape::TS_SPHEROID);
  exports.set("SNUBNOSE", OpenSCAM::ToolShape::TS_SNUBNOSE);

#undef XYZ
#undef ABC
#undef UVW
#undef IJK
#undef AXES
}


js::Value GCodeModule::gcodeCB(const js::Arguments &args) {
  string path =
    SystemUtilities::absolute(ctx.getCurrentPath(), args.getString("path"));

  ctx.pushPath(path);
  SmartFunctor<TPLContext> popPath(&ctx, &TPLContext::popPath);

  Options options;
  OpenSCAM::Controller controller(ctx.machine);
  OpenSCAM::Interpreter(controller).read(path);

  return js::Value();
}


js::Value GCodeModule::rapidCB(const js::Arguments &args) {
  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes, args.getBoolean("incremental"));
  ctx.machine.move(axes, true);

  return js::Value();
}


js::Value GCodeModule::cutCB(const js::Arguments &args) {
  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes, args.getBoolean("incremental"));
  ctx.machine.move(axes);

  return js::Value();
}


js::Value GCodeModule::arcCB(const js::Arguments &args) {
  // TODO Handle 'incremental=false'

  Vector3D
    offset(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"));
  double angle = args.getNumber("angle");
  plane_t plane = args.has("plane") ? (plane_t)args.getUint32("plane") : XY;

  ctx.machine.arc(offset, angle, plane);

  return js::Value();
}


js::Value GCodeModule::probeCB(const js::Arguments &args) {
  bool toward = args.getBoolean("toward");
  bool error = args.getBoolean("error");
  uint32_t index = args.getUint32("index");
  int port = args.getInt32("port");
  bool invert = args.getInt32("invert");

  if (port == -1) port = ctx.machine.findPort(PROBE, index);
  if (port != -1)
    ctx.machine.input(port, toward ^ invert ? STOP_WHEN_HIGH : STOP_WHEN_LOW,
                  error);

  Axes axes = ctx.machine.getPosition();
  parseAxes(args, axes);
  ctx.machine.move(axes);

  return js::Value();
}


js::Value GCodeModule::dwellCB(const js::Arguments &args) {
  ctx.machine.dwell(args["seconds"].toNumber());
  return js::Value();
}


js::Value GCodeModule::feedCB(const js::Arguments &args) {
  // Return feed info if no arguments were given
  if (!args.getCount()) {
    js::Value array = js::Value::createArray(2);

    feed_mode_t mode;
    array.set(0, ctx.machine.getFeed(&mode));
    array.set(1, mode);

    return array;
  }

  // Otherwise set feed
  feed_mode_t mode = MM_PER_MINUTE;
  if (args.has("mode")) {
    mode = (feed_mode_t)args["mode"].toUint32();
    switch (mode) {
    case INVERSE_TIME: case MM_PER_MINUTE: case MM_PER_REVOLUTION: break;
    default: THROW("Feed mode must be FEED_INVERSE_TIME, FEED_UNITS_PER_MIN or "
                   "FEED_UNITS_PER_REV");
    }
  }

  ctx.machine.setFeed(args["rate"].toNumber(), mode);

  return ctx.machine.getFeed();
}


js::Value GCodeModule::speedCB(const js::Arguments &args) {
  // Return spindle info if no arguments were given
  if (!args.getCount()) {
    js::Value array = js::Value::createArray(3);

    spin_mode_t mode;
    double max;
    array.set(0, ctx.machine.getSpeed(&mode, &max));
    array.set(1, mode);
    array.set(2, max);

    return array;
  }

  // Otherwise set spindle
  spin_mode_t mode = REVOLUTIONS_PER_MINUTE;
  double max = 0;

  if (args.has("surface")) {
    mode = CONSTANT_SURFACE_SPEED;
    if (args.has("max")) max = args["max"].toNumber();
  }

  ctx.machine.setSpeed(args["rate"].toNumber(), mode, max);

  return ctx.machine.getSpeed();
}


js::Value GCodeModule::toolCB(const js::Arguments &args) {
  // Return tool number if no arguments were given
  if (!args.getCount()) return ctx.machine.getTool();

  uint32_t number = args["number"].toUint32();
  ctx.tools->get(number); // Make sure it exists
  ctx.machine.setTool(number);

  return ctx.machine.getTool();
}


js::Value GCodeModule::unitsCB(const js::Arguments &args) {
  MachineUnitAdapter::units_t units = unitAdapter.getUnits();

  if (args.has("type"))
    switch (args["type"].toUint32()) {
    case MachineUnitAdapter::IMPERIAL:
      unitAdapter.setUnits(MachineUnitAdapter::IMPERIAL);
      break;
    case MachineUnitAdapter::METRIC:
      unitAdapter.setUnits(MachineUnitAdapter::METRIC);
      break;
    default: THROWS("Units type must be one of IMPERIAL or METRIC");
    }
  else return js::Value(units);

  return js::Value();
}


js::Value GCodeModule::pauseCB(const js::Arguments &args) {
  ctx.machine.pause(args["optional"].toBoolean());
  return js::Value();
}


js::Value GCodeModule::toolSetCB(const js::Arguments &args) {
  SmartPointer<OpenSCAM::Tool> tool = ctx.tools->get(args["number"].toUint32());

  uint32_t units;
  double scale = 1;
  if (args.has("units")) units = args["units"].toUint32();
  else units = unitAdapter.getUnits();
  if (units == MachineUnitAdapter::METRIC)
    tool->setUnits(OpenSCAM::ToolUnits::UNITS_MM);
  else {
    tool->setUnits(OpenSCAM::ToolUnits::UNITS_INCH);
    scale = 25.4;
  }

  if (args.has("shape"))
    tool->setShape((OpenSCAM::ToolShape::enum_t)args["shape"].toUint32());

  tool->setLength(scale * args.getNumber("length"));
  tool->setDiameter(scale * args.getNumber("diameter"));
  tool->setSnubDiameter(scale * args.getNumber("snub"));
  tool->setFrontAngle(args.getNumber("front_angle"));
  tool->setBackAngle(args.getNumber("back_angle"));
  tool->setOrientation(args.getNumber("orientation"));

  return js::Value();
}


js::Value GCodeModule::positionCB(const js::Arguments &args) {
  Axes axes = ctx.machine.getPosition();

  js::Value obj = js::Value::createObject();

  for (unsigned i = 0; Axes::AXES[i]; i++)
    obj.set(string(1, tolower(Axes::AXES[i])), axes.getIndex(i));

  return obj;
}


void GCodeModule::parseAxes(const js::Arguments &args, Axes &axes,
                             bool incremental) {
  for (const char *axis ="xyzabcuvw"; *axis; axis++) {
    string name = string(1, *axis);
    if (args.has(name)) axes.set(*axis, args.getNumber(name) +
                                 (incremental ? axes.get(*axis) : 0));
  }
}
