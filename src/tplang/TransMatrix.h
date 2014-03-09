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

#ifndef TPLANG_TRANS_MATRIX_H
#define TPLANG_TRANS_MATRIX_H

#include <cbang/geom/Matrix.h>

namespace tplang {
  class TransMatrix {
    cb::Matrix4x4D m;
    cb::Matrix4x4D i;

  public:
    TransMatrix();

    const cb::Matrix4x4D &getMatrix() const {return m;}
    void setMatrix(const cb::Matrix4x4D &m);

    const cb::Matrix4x4D &getInverse() const {return i;}
    void setInverse(const cb::Matrix4x4D &i);

    void identity();
    void scale(const cb::Vector3D &o);
    void translate(const cb::Vector3D &o);
    void rotate(double angle, const cb::Vector3D &o);
    void reflect(const cb::Vector3D &o);

    cb::Vector3D transform(const cb::Vector3D &p) const;
    cb::Vector3D invert(const cb::Vector3D &p) const;
  };
}

#endif // TPLANG_TRANS_MATRIX_H

