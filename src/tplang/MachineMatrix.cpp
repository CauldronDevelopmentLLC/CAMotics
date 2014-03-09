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

#include "MachineMatrix.h"

#include <cbang/Exception.h>
#include <cbang/log/Logger.h>

using namespace tplang;
using namespace cb;


MachineMatrix::MachineMatrix() {
  for (unsigned i = 0; i < 3; i++) matrices[i].push_back(TransMatrix());
}


void MachineMatrix::pushMatrix(axes_t matrix) {
  matrices_t &m = getMatrices(matrix);
  m.push_back(m.back());
}


void MachineMatrix::popMatrix(axes_t matrix) {
  matrices_t &matrices = getMatrices(matrix);
  if (matrices.size() == 1) THROW("Matrix stack empty");
  matrices.pop_back();

  updateMatrix(matrix);
}


void MachineMatrix::loadIdentity(axes_t matrix) {
  getTransMatrix(matrix).identity();
  updateMatrix(matrix);
}


void MachineMatrix::scale(double x, double y, double z, axes_t matrix) {
  getTransMatrix(matrix).scale(Vector3D(x, y, z));
  updateMatrix(matrix);
}


void MachineMatrix::translate(double x, double y, double z, axes_t matrix) {
  // TODO Need to convert imperial units to metric
  getTransMatrix(matrix).translate(Vector3D(x, y, z));
  updateMatrix(matrix);
}


void MachineMatrix::rotate(double angle, double x, double y, double z,
                           axes_t matrix) {
  if (!angle) return;
  getTransMatrix(matrix).rotate(angle, Vector3D(x, y, z));
  updateMatrix(matrix);
}


void MachineMatrix::reflect(double x, double y, double z, axes_t matrix) {
  getTransMatrix(matrix).reflect(Vector3D(x, y, z));
  updateMatrix(matrix);
}


void MachineMatrix::start() {
  loadIdentity(XYZ);
  loadIdentity(ABC);
  loadIdentity(UVW);

  MachineAdapter::start();
}


Axes MachineMatrix::getPosition() const {
  Axes axes = MachineAdapter::getPosition();

  // TODO this is inefficient
  axes.setXYZ(getTransMatrix(XYZ).invert(axes.getXYZ()));
  axes.setABC(getTransMatrix(ABC).invert(axes.getABC()));
  axes.setUVW(getTransMatrix(UVW).invert(axes.getUVW()));

  return axes;
}


Vector3D MachineMatrix::getPosition(axes_t axes) const {
  return getTransMatrix(axes).invert(MachineAdapter::getPosition(axes));
}


void MachineMatrix::move(const Axes &axes, bool rapid) {
  Axes trans(axes);

  // TODO this is inefficient
  trans.applyXYZMatrix(getMatrix(XYZ));
  trans.applyABCMatrix(getMatrix(ABC));
  trans.applyUVWMatrix(getMatrix(UVW));

  MachineAdapter::move(trans, rapid);
}


void MachineMatrix::arc(const cb::Vector3D &offset, double angle,
                        plane_t plane) {
  THROW("MachineMatrix cannot handle arc directly");
}


void MachineMatrix::setMatrix(const Matrix4x4D &t, axes_t matrix) {
  getTransMatrix(matrix).setMatrix(t);
  updateMatrix(matrix);
}


MachineMatrix::matrices_t &MachineMatrix::getMatrices(axes_t matrix) {
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix " << matrix);
  return matrices[matrix];
}


const MachineMatrix::matrices_t &
MachineMatrix::getMatrices(axes_t matrix) const {
  if (AXES_COUNT <= matrix) THROWS("Invalid matrix " << matrix);
  return matrices[matrix];
}


TransMatrix &MachineMatrix::getTransMatrix(axes_t matrix) {
  matrices_t &matrices = getMatrices(matrix);
  if (matrices.empty()) THROW("Matrix stack empty");
  return matrices.back();
}


const TransMatrix &MachineMatrix::getTransMatrix(axes_t matrix) const {
  const matrices_t &matrices = getMatrices(matrix);
  if (matrices.empty()) THROW("Matrix stack empty");
  return matrices.back();
}


void MachineMatrix::updateMatrix(axes_t matrix) {
  const Matrix4x4D &m = getTransMatrix(matrix).getMatrix();

  if (MachineAdapter::getMatrix(matrix) != m)
    MachineAdapter::setMatrix(m, matrix);
}
