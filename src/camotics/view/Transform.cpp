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

using namespace CAMotics;
using namespace cb;


void Transform::perspective(double fovy, double aspect, double zNear,
                            double zFar) {
  double a = fovy / 2;
  double f = cos(a) / sin(a);
  Matrix4x4D m;

  m[0][0] = f / aspect;
  m[0][1] = 0;
  m[0][2] = 0;
  m[0][3] = 0;

  m[1][0] = 0;
  m[1][1] = f;
  m[1][2] = 0;
  m[1][3] = 0;

  m[2][0] = 0;
  m[2][1] = 0;
  m[2][2] = (zFar + zNear) / (zNear - zFar);
  m[2][3] = 2 * zFar * zNear / (zNear - zFar);

  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = -1;
  m[3][3] = 0;

  *this *= m;
}


void Transform::translate(double x, double y, double z) {
  Matrix4x4D m;

  m.toIdentity();
  m[0][3] = x;
  m[1][3] = y;
  m[2][3] = z;

  *this *= m;
}


void Transform::translate(const Vector3D &v) {translate(v.x(), v.y(), v.z());}


void Transform::rotate(double angle, const Vector3D &_v) {
  Matrix4x4D m;
  Vector3D v = _v.normalize();
  double x = v.x();
  double y = v.y();
  double z = v.z();
  double c = cos(angle);
  double s = sin(angle);

  if (!v.isReal()) return;

  m.toIdentity();

  m[0][0] = x * x * (1 - c) + c;
  m[0][1] = x * y * (1 - c) - z * s;
  m[0][2] = x * z * (1 - c) + y * s;
  m[0][3] = 0;

  m[1][0] = y * x * (1 - c) + z * s;
  m[1][1] = y * y * (1 - c) + c;
  m[1][2] = y * z * (1 - c) - x * s;
  m[1][3] = 0;

  m[2][0] = x * z * (1 - c) - y * s;
  m[2][1] = y * z * (1 - c) + x * s;
  m[2][2] = z * z * (1 - c) + c;
  m[2][3] = 0;

  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = 0;
  m[3][3] = 1;

  *this *= m;
}


void Transform::rotate(double angle, double x, double y, double z) {
  rotate(angle, Vector3D(x, y, z));
}


void Transform::rotate(const QuaternionD &q) {
  AxisAngleD a = q.toAxisAngle();
  rotate(a.angle(), a.getVector());
}


void Transform::scale(double x, double y, double z) {
  Matrix4x4D m;
  m.toIdentity();

  m[0][0] = x;
  m[1][1] = y;
  m[2][2] = z;

  *this *= m;
}


void Transform::scale(const Vector3D &v) {scale(v.x(), v.y(), v.z());}


void Transform::lookAt(const Vector3D &eye, const Vector3D &center,
                       const Vector3D &up) {
  Vector3D f = (center - eye).normalize();
  Vector3D s = f.cross(up.normalize()).normalize();
  Vector3D u = s.normalize().cross(f).normalize();

  Matrix4x4D m;
  m[0][0] = s[0];
  m[0][1] = s[1];
  m[0][2] = s[2];
  m[0][3] = 0;

  m[1][0] = u[0];
  m[1][1] = u[1];
  m[1][2] = u[2];
  m[1][3] = 0;

  m[2][0] = -f[0];
  m[2][1] = -f[1];
  m[2][2] = -f[2];
  m[2][3] = 0;

  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = 0;
  m[3][3] = 1;

  *this *= m;

  // Translate eye to origin
  translate(-eye);
}
