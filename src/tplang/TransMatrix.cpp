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

#include "TransMatrix.h"

using namespace tplang;
using namespace cb;


TransMatrix::TransMatrix() {
  identity();
}


void TransMatrix::setMatrix(const Matrix4x4D &m) {
  this->m = m;
  i = m;
  i.inverse();
}


void TransMatrix::setInverse(const Matrix4x4D &i) {
  this->i = i;
  m = i;
  m.inverse();
}


void TransMatrix::identity() {
  m.toIdentity();
  i.toIdentity();
}


void TransMatrix::scale(const Vector3D &o) {
  Matrix4x4D t;

  for (unsigned j = 0; j < 3; j++) {
    if (!o[j]) THROW("Cannot scale by zero");
    t[j][j] = o[j];
  }
  t[3][3] = 1;

  m = t * m;

  for (unsigned j = 0; j < 3; j++)
    t[j][j] = 1.0 / o[j];

  i = i * t;
}


void TransMatrix::translate(const Vector3D &o) {
  Matrix4x4D t;

  for (unsigned j = 0; j < 3; j++) {
    t[j][3] = o[j];
    t[j][j] = 1;
  }
  t[3][3] = 1;

  m = t * m;

  for (unsigned j = 0; j < 3; j++)
    t[j][3] = -o[j];

  i = i * t;
}


namespace {
  void makeRotationMatrix(Matrix4x4D &r, double angle, const Vector3D &v) {
    double c = cos(angle);
    double s = sin(angle);
    double l = 1.0 - c;

    r[0][0] = c + v.x() * v.x() * l;
    r[1][1] = c + v.y() * v.y() * l;
    r[2][2] = c + v.z() * v.z() * l;
    r[3][3] = 1;

    double tmp1 = v.x() * v.y() * l;
    double tmp2 = v.z() * s;
    r[1][0] = tmp1 + tmp2;
    r[0][1] = tmp1 - tmp2;
    tmp1 = v.x() * v.z() * l;
    tmp2 = v.y() * s;
    r[2][0] = tmp1 - tmp2;
    r[0][2] = tmp1 + tmp2;
    tmp1 = v.y() * v.z() * l;
    tmp2 = v.x() * s;
    r[2][1] = tmp1 + tmp2;
    r[1][2] = tmp1 - tmp2;
  }
}


void TransMatrix::rotate(double angle, const Vector3D &o) {
  if (!angle) return;
  if (!o[0] && !o[1] && !o[2]) THROW("Invalid rotation axis (0,0,0)");

  Vector3D v = o;
  v = v.normalize();

  Matrix4x4D t;

  makeRotationMatrix(t, angle, v);
  m = t * m;
  makeRotationMatrix(t, -angle, v);
  i = i * t;
}


void TransMatrix::reflect(const Vector3D &o) {
  Matrix4x4D t;

  t[0][0] = 1 - 2 * o[0] * o[0];
  t[0][1] = t[1][0] = -2 * o[0] * o[1];
  t[0][2] = t[2][0] = -2 * o[0] * o[2];
  t[1][1] = 1 - 2 * o[1] * o[1];
  t[1][2] = t[2][1] = -2 * o[1] * o[2];
  t[2][2] = 1 - 2 * o[2] * o[2];
  t[3][3] = 1;

  m = t * m;
  i = i * t;
}


Vector3D TransMatrix::transform(const Vector3D &p) const {
  return (m * Vector4D(p, 1)).slice<3>();
}


Vector3D TransMatrix::invert(const Vector3D &p) const {
  return (i * Vector4D(p, 1)).slice<3>();
}
