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
#include <cbang/json/Serializable.h>
#include <cbang/js/Value.h>


namespace GCode {
  class Transform : public cb::Matrix4x4D, public cb::JSON::Serializable {
  public:
    Transform(const cb::Matrix4x4D &m) : cb::Matrix4x4D(m) {}
    Transform(const cb::js::Value &value)   {read(value);}
    Transform(const cb::JSON::Value &value) {read(value);}
    Transform() {toIdentity();}

    void scale(const cb::Vector3D &o);
    void translate(const cb::Vector3D &o);
    void rotate(double angle, const cb::Vector3D &o, const cb::Vector3D &u);
    void reflect(const cb::Vector3D &o);

    cb::Vector3D transform(const cb::Vector3D &p) const;

    void read(const cb::js::Value &value);
    using cb::JSON::Serializable::read;
    using cb::JSON::Serializable::write;

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
