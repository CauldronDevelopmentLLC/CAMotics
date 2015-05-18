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

#include "MatrixModule.h"

using namespace cb;
using namespace tplang;


void MatrixModule::define(js::ObjectTemplate &exports) {
  exports.set("pushMatrix(matrix)", this, &MatrixModule::pushMatrixCB);
  exports.set("popMatrix(matrix)", this, &MatrixModule::popMatrixCB);
  exports.set("loadIdentity(matrix)", this, &MatrixModule::loadIdentityCB);
  exports.set("scale(x=1, y=1, z=1, matrix)", this, &MatrixModule::scaleCB);
  exports.set("translate(x=0, y=0, z=0, matrix)", this,
           &MatrixModule::translateCB);
  exports.set("rotate(angle, x=0, y=0, z=1, matrix)", this,
           &MatrixModule::rotateCB);
  exports.set("setMatrix(m, matrix)", this, &MatrixModule::setMatrixCB);
  exports.set("getMatrix(m)", this, &MatrixModule::getMatrixCB);

  // TODO Consider replacing these with get(X), get(Y), etc.
  exports.set("getXYZ()", this, &MatrixModule::getXYZ);
  exports.set("getX()", this, &MatrixModule::getX);
  exports.set("getY()", this, &MatrixModule::getY);
  exports.set("getZ()", this, &MatrixModule::getZ);
}


js::Value MatrixModule::pushMatrixCB(const js::Arguments &args) {
  matrix.pushMatrix(parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::popMatrixCB(const js::Arguments &args) {
  matrix.popMatrix(parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::loadIdentityCB(const js::Arguments &args) {
  matrix.loadIdentity(parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::scaleCB(const js::Arguments &args) {
  matrix.scale(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"),
               parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::translateCB(const js::Arguments &args) {
  matrix.translate(args.getNumber("x"), args.getNumber("y"),
                   args.getNumber("z"), parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::rotateCB(const js::Arguments &args) {
  matrix.rotate(args.getNumber("angle"), args.getNumber("x"),
                args.getNumber("y"), args.getNumber("z"), parseMatrix(args));
  return js::Value();
}


js::Value MatrixModule::setMatrixCB(const js::Arguments &args) {
  THROW("Not yet implemented");
  return js::Value();
}


js::Value MatrixModule::getMatrixCB(const js::Arguments &args) {
  THROW("Not yet implemented");
  return js::Value();
}


MatrixModule::axes_t MatrixModule::parseMatrix(const js::Arguments &args) {
  if (!args.has("matrix")) return XYZ;

  axes_t matrix = (axes_t)args["matrix"].toUint32();
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix number " << matrix);

  return matrix;
}


js::Value MatrixModule::getXYZ(const js::Arguments &args) {
  Vector3D v = ctx.machine.getPosition(XYZ);
  js::Value array = js::Value::createArray();

  array.set(0, v.x());
  array.set(1, v.y());
  array.set(2, v.z());

  return array;
}


js::Value MatrixModule::getX(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).x();
}


js::Value MatrixModule::getY(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).y();
}


js::Value MatrixModule::getZ(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).z();
}
