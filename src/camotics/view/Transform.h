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

#pragma once


#include <cbang/geom/Matrix.h>
#include <cbang/geom/Quaternion.h>


namespace CAMotics {
  class Transform : public cb::Matrix4x4D {
  public:
    Transform() {toIdentity();}
    Transform(const cb::Matrix4x4D &m) : cb::Matrix4x4D(m) {}

    void perspective(double fovy, double aspect, double zNear, double zFar);
    void translate(double x, double y, double z);
    void translate(const cb::Vector3D &v);
    void rotate(double angle, const cb::Vector3D &v);
    void rotate(double angle, double x, double y, double z);
    void rotate(const cb::QuaternionD &q);
    void scale(double x, double y, double z);
    void scale(const cb::Vector3D &v);
    void lookAt(const cb::Vector3D &eye, const cb::Vector3D &center,
                const cb::Vector3D &up);
    void invert();
    void transpose() {inplaceTranspose();}
    cb::Matrix3x3D upper3x3() const;
  };
}
