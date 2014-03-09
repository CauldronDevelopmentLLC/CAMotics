/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "MatrixLibrary.h"

using namespace cb;
using namespace tplang;


void MatrixLibrary::add(js::ObjectTemplate &tmpl) {
  tmpl.set("pushMatrix(matrix)", this, &MatrixLibrary::pushMatrixCB);
  tmpl.set("popMatrix(matrix)", this, &MatrixLibrary::popMatrixCB);
  tmpl.set("loadIdentity(matrix)", this, &MatrixLibrary::loadIdentityCB);
  tmpl.set("scale(x=1, y=1, z=1, matrix)", this, &MatrixLibrary::scaleCB);
  tmpl.set("translate(x=0, y=0, z=0, matrix)", this,
           &MatrixLibrary::translateCB);
  tmpl.set("rotate(angle, x=0, y=0, z=1, matrix)", this,
           &MatrixLibrary::rotateCB);
  tmpl.set("setMatrix(m, matrix)", this, &MatrixLibrary::setMatrixCB);
  tmpl.set("getMatrix(m)", this, &MatrixLibrary::getMatrixCB);

  // TODO Consider replacing these with get(X), get(Y), etc.
  tmpl.set("getXYZ()", this, &MatrixLibrary::getXYZ);
  tmpl.set("getX()", this, &MatrixLibrary::getX);
  tmpl.set("getY()", this, &MatrixLibrary::getY);
  tmpl.set("getZ()", this, &MatrixLibrary::getZ);
}


js::Value MatrixLibrary::pushMatrixCB(const js::Arguments &args) {
  matrix.pushMatrix(parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::popMatrixCB(const js::Arguments &args) {
  matrix.popMatrix(parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::loadIdentityCB(const js::Arguments &args) {
  matrix.loadIdentity(parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::scaleCB(const js::Arguments &args) {
  matrix.scale(args.getNumber("x"), args.getNumber("y"), args.getNumber("z"),
               parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::translateCB(const js::Arguments &args) {
  matrix.translate(args.getNumber("x"), args.getNumber("y"),
                   args.getNumber("z"), parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::rotateCB(const js::Arguments &args) {
  matrix.rotate(args.getNumber("angle"), args.getNumber("x"),
                args.getNumber("y"), args.getNumber("z"), parseMatrix(args));
  return js::Value();
}


js::Value MatrixLibrary::setMatrixCB(const js::Arguments &args) {
  THROW("Not yet implemented");
  return js::Value();
}


js::Value MatrixLibrary::getMatrixCB(const js::Arguments &args) {
  THROW("Not yet implemented");
  return js::Value();
}


MatrixLibrary::axes_t MatrixLibrary::parseMatrix(const js::Arguments &args) {
  if (!args.has("matrix")) return XYZ;

  axes_t matrix = (axes_t)args["matrix"].toUint32();
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix number " << matrix);

  return matrix;
}


js::Value MatrixLibrary::getXYZ(const js::Arguments &args) {
  Vector3D v = ctx.machine.getPosition(XYZ);
  js::Value array = js::Value::createArray();

  array.set(0, v.x());
  array.set(1, v.y());
  array.set(2, v.z());

  return array;
}


js::Value MatrixLibrary::getX(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).x();
}


js::Value MatrixLibrary::getY(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).y();
}


js::Value MatrixLibrary::getZ(const js::Arguments &args) {
  return ctx.machine.getPosition(XYZ).z();
}
