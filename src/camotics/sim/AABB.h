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

#include <gcode/Move.h>

#include <cbang/geom/Rectangle.h>

#include <vector>


namespace CAMotics {
  class AABB : public cb::Rectangle3D {
    AABB *left;
    AABB *right;
    const GCode::Move *move;

  public:
    AABB(AABB *nodes);
    AABB(const GCode::Move *move, const cb::Rectangle3D &bbox) :
      cb::Rectangle3D(bbox), left(0), right(0), move(move) {}
    ~AABB();

    const AABB *getLeft() const {return left;}
    const AABB *getRight() const {return right;}

    AABB *prepend(AABB *list);
    AABB *split(unsigned count);

    cb::Rectangle3D getBounds() const {return *this;}
    const GCode::Move *getMove() const {return move;}
    bool isLeaf() const {return move;}
    unsigned getTreeHeight() const;

    bool intersects(const cb::Rectangle3D &r);
    void collisions(const cb::Vector3D &p,
                    std::vector<const GCode::Move *> &moves);
  };
}
