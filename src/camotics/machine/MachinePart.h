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

#include <camotics/contour/TriangleSurface.h>

#include <cbang/geom/Triangle.h>
#include <cbang/geom/Rectangle.h>
#include <cbang/geom/Matrix.h>
#include <cbang/io/InputSource.h>
#include <cbang/json/Value.h>

#include <vector>

namespace CAMotics {
  class MachinePart : public TriangleSurface {
    const std::string name;
    cb::Vector3U color;

    cb::Vector3D init;
    cb::Vector3D home;
    cb::Vector3D min;
    cb::Vector3D max;
    cb::Vector3D movement;
    cb::Vector3D offset;
    std::vector<float> lines;

  public:
    MachinePart(const std::string &name,
                cb::SmartPointer<cb::JSON::Value> &config);

    const cb::Vector3U &getColor() const {return color;}
    const cb::Vector3D &getInit() const {return init;}
    const cb::Vector3D &getHome() const {return home;}
    const cb::Vector3D &getMin() const {return min;}
    const cb::Vector3D &getMax() const {return max;}

    const cb::Vector3D &getMovement() const {return movement;}
    const cb::Vector3D &getOffset() const {return offset;}
    const std::vector<float> getLines() const {return lines;}

    void read(const cb::InputSource &source, const cb::Matrix4x4D &transform,
              bool reverseWinding);
    using TriangleSurface::read;
  };
}
