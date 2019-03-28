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

#include "MatrixModule.h"
#include "TPLContext.h"

using namespace cb;
using namespace tplang;


MatrixModule::MatrixModule(TPLContext &ctx) :
  js::NativeModule("matrix"), ctx(ctx) {}


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


GCode::TransformStack &MatrixModule::getTransformStack(const js::Value &args) {
  return ctx.getMachine().getTransforms().get(parseAxes(args));
}


GCode::Transform &MatrixModule::getTransform(const js::Value &args) {
  return getTransformStack(args).top();
}


void MatrixModule::pushMatrixCB(const js::Value &args, js::Sink &sink) {
  getTransformStack(args).push();
}


void MatrixModule::popMatrixCB(const js::Value &args, js::Sink &sink) {
  getTransformStack(args).pop();
}


void MatrixModule::loadIdentityCB(const js::Value &args, js::Sink &sink) {
  getTransform(args).toIdentity();
}


void MatrixModule::scaleCB(const js::Value &args, js::Sink &sink) {
  Vector3D v(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"));
  getTransform(args).scale(v);
}


void MatrixModule::translateCB(const js::Value &args, js::Sink &sink) {
  Vector3D v(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"));
  getTransform(args).translate(v);
}


void MatrixModule::rotateCB(const js::Value &args, js::Sink &sink) {
  Vector3D v1(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"));
  Vector3D v2(args.getNumber("a"), args.getNumber("b"), args.getNumber("c"));
  getTransform(args).rotate(args.getNumber("angle"), v1, v2);
}


void MatrixModule::setMatrixCB(const js::Value &args, js::Sink &sink) {
  getTransform(args) = GCode::Transform(*args.get("m"));
}


void MatrixModule::getMatrixCB(const js::Value &args, js::Sink &sink) {
  getTransform(args).write(sink);
}


MatrixModule::axes_t MatrixModule::parseAxes(const js::Value &args) {
  if (!args.has("matrix")) return XYZ;

  axes_t matrix = (axes_t)args.getInteger("matrix");
  if (AXES_COUNT <= matrix) THROW("Invalid matrix number " << matrix);

  return matrix;
}


void MatrixModule::getXYZ(const js::Value &args, js::Sink &sink) {
  Vector3D v = ctx.getMachine().getPosition(XYZ);

  sink.beginList();
  sink.append(v.x());
  sink.append(v.y());
  sink.append(v.z());
  sink.endList();
}


void MatrixModule::getX(const js::Value &args, js::Sink &sink) {
  sink.write(ctx.getMachine().getPosition(XYZ).x());
}


void MatrixModule::getY(const js::Value &args, js::Sink &sink) {
  sink.write(ctx.getMachine().getPosition(XYZ).y());
}


void MatrixModule::getZ(const js::Value &args, js::Sink &sink) {
  sink.write(ctx.getMachine().getPosition(XYZ).z());
}
