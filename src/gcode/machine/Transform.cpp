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

#include "Transform.h"

using namespace GCode;
using namespace cb;


void Transform::scale(const Vector3D &o) {
  Matrix4x4D t;

  for (unsigned j = 0; j < 3; j++) {
    if (!o[j]) THROW("Cannot scale by zero");
    t[j][j] = o[j];
  }
  t[3][3] = 1;

  *this = t * *this;
}


void Transform::translate(const Vector3D &o) {
  Matrix4x4D t;

  for (unsigned j = 0; j < 3; j++) {
    t[j][3] = o[j];
    t[j][j] = 1;
  }
  t[3][3] = 1;

  *this = *this * t;
}


namespace {
  void makeRotationMatrix(Matrix4x4D &r, double angle, const Vector3D &m,
                          const Vector3D &n) {
    double co = cos(angle);
    double si = sin(angle);
    double l = 1.0 - co;
    double u = m.x();
    double v = m.y();
    double w = m.z();
    double a = n.x();
    double b = n.y();
    double c = n.z();

    r[0][0] = u * u + (v * v + w * w) * co;
    r[0][1] = u * v * l - w * si;
    r[0][2] = u * w * l + v * si;
    r[0][3] =
      (a * (v * v + w * w) - u * (b * v + c * w)) * l + (b * w - c * v) * si;
    r[1][0] = u * v * l + w * si;
    r[1][1] = v * v + (u * u + w * w) * co;
    r[1][2] = v * w * l - u * si;
    r[1][3] =
      (b * (u * u + w * w) - v * (a * u + c * w)) * l + (c * u - a * w) * si;
    r[2][0] = u * w * l - v * si;
    r[2][1] = v * w * l + u * si;
    r[2][2] = w * w + (u * u + v * v) * co;
    r[2][3] =
      (c * (u * u + v * v) - w * (a * u + b * v)) * l + (a * v - b * u) * si;
    r[3][0] = 0;
    r[3][1] = 0;
    r[3][2] = 0;
    r[3][3] = 1;
  }
}


void Transform::rotate(double angle, const Vector3D &_v, const Vector3D &_u) {
  if (!angle) return;
  if (!_v[0] && !_v[1] && !_v[2]) THROW("Invalid rotation axis (0,0,0)");

  Vector3D v = _v;
  v = v.normalize();
  Vector3D u = transform(_u);

  Matrix4x4D t;

  makeRotationMatrix(t, angle, v, u);
  *this = t * *this;
}


void Transform::reflect(const Vector3D &_o) {
  Matrix4x4D t;

  Vector3D o = transform(_o);

  t[0][0] = 1 - 2 * o[0] * o[0];
  t[0][1] = t[1][0] = -2 * o[0] * o[1];
  t[0][2] = t[2][0] = -2 * o[0] * o[2];
  t[1][1] = 1 - 2 * o[1] * o[1];
  t[1][2] = t[2][1] = -2 * o[1] * o[2];
  t[2][2] = 1 - 2 * o[2] * o[2];
  t[3][3] = 1;

  *this = t * *this;
}


Vector3D Transform::transform(const Vector3D &p) const {
  return (*this * Vector4D(p, 1)).slice<3>();
}


void Transform::read(const js::Value &value) {
  if (!value.isArray() || value.length() != 4)
    THROW("Transform expected 4 rows");

  for (unsigned i = 0; i < 4; i++) {
    SmartPointer<js::Value> row = value.get(i);
    if (!row->isArray() || row->length() != 4)
      THROW("Transform expected row with 4 columns");

    for (unsigned j = 0; j < 4; j++)
      data[i][j] = row->getNumber(j);
  }
}


void Transform::read(const JSON::Value &value) {
  auto &rows = value.getList();

  if (rows.size() != 4) THROW("Transform expected 4 rows");

  for (unsigned i = 0; i < 4; i++) {
    auto &row = rows.getList(i);
    if (row.size() != 4) THROW("Transform expected row with 4 columns");

    for (unsigned j = 0; j < 4; j++)
      data[i][j] = row.getNumber(j);
  }
}


void Transform::write(JSON::Sink &sink) const {
  sink.beginList();

  for (unsigned i = 0; i < 4; i++) {
    sink.appendList(true);

    for (unsigned j = 0; j < 4; j++)
      sink.append(data[i][j]);

    sink.endList();
  }

  sink.endList();
}
