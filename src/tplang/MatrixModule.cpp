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

#include "MatrixModule.h"
#include "TPLContext.h"

using namespace cb;
using namespace tplang;


MatrixModule::MatrixModule(TPLContext &ctx) : ctx(ctx), matrix(0) {}


void MatrixModule::define(js::Sink &exports) {
  exports.insert("pushMatrix(matrix)", this, &MatrixModule::pushMatrixCB);
  exports.insert("popMatrix(matrix)", this, &MatrixModule::popMatrixCB);
  exports.insert("loadIdentity(matrix)", this, &MatrixModule::loadIdentityCB);
  exports.insert("scale(x=1, y=1, z=1, matrix)", this, &MatrixModule::scaleCB);
  exports.insert("translate(x=0, y=0, z=0, matrix)", this,
              &MatrixModule::translateCB);
  exports.insert("rotate(angle, x=0, y=0, z=1, a=0, b=0, c=0, matrix)", this,
              &MatrixModule::rotateCB);
  exports.insert("setMatrix(m, matrix)", this, &MatrixModule::setMatrixCB);
  exports.insert("getMatrix(m)", this, &MatrixModule::getMatrixCB);

  // TODO Consider replacing these with get(X), get(Y), etc.
  exports.insert("getXYZ()", this, &MatrixModule::getXYZ);
  exports.insert("getX()", this, &MatrixModule::getX);
  exports.insert("getY()", this, &MatrixModule::getY);
  exports.insert("getZ()", this, &MatrixModule::getZ);
}


CAMotics::MachineMatrix &MatrixModule::getMatrix() {
  if (!matrix) matrix = &ctx.find<CAMotics::MachineMatrix>();
  return *matrix;
}


void MatrixModule::pushMatrixCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().pushMatrix(parseMatrix(args));
}


void MatrixModule::popMatrixCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().popMatrix(parseMatrix(args));
}


void MatrixModule::loadIdentityCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().loadIdentity(parseMatrix(args));
}


void MatrixModule::scaleCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().scale(args.getNumber("x"), args.getNumber("y"),
                    args.getNumber("z"), parseMatrix(args));
}


void MatrixModule::translateCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().translate(args.getNumber("x"), args.getNumber("y"),
                        args.getNumber("z"), parseMatrix(args));
}


void MatrixModule::rotateCB(const JSON::Value &args, js::Sink &sink) {
  getMatrix().rotate(args.getNumber("angle"), args.getNumber("x"),
                     args.getNumber("y"), args.getNumber("z"),
                     args.getNumber("a"), args.getNumber("b"),
                     args.getNumber("c"), parseMatrix(args));
}


void MatrixModule::setMatrixCB(const JSON::Value &args, js::Sink &sink) {
  THROW("Not yet implemented");
}


void MatrixModule::getMatrixCB(const JSON::Value &args, js::Sink &sink) {
  THROW("Not yet implemented");
}


MatrixModule::axes_t MatrixModule::parseMatrix(const JSON::Value &args) {
  if (!args.has("matrix")) return XYZ;

  axes_t matrix = (axes_t)args.getU32("matrix");
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix number " << matrix);

  return matrix;
}


void MatrixModule::getXYZ(const JSON::Value &args, js::Sink &sink) {
  Vector3D v = ctx.machine.getPosition(XYZ);

  sink.beginList();
  sink.append(v.x());
  sink.append(v.y());
  sink.append(v.z());
  sink.endList();
}


void MatrixModule::getX(const JSON::Value &args, js::Sink &sink) {
  sink.write(ctx.machine.getPosition(XYZ).x());
}


void MatrixModule::getY(const JSON::Value &args, js::Sink &sink) {
  sink.write(ctx.machine.getPosition(XYZ).y());
}


void MatrixModule::getZ(const JSON::Value &args, js::Sink &sink) {
  sink.write(ctx.machine.getPosition(XYZ).z());
}
