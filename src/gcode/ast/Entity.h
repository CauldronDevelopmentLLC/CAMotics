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


#include <gcode/parse/Token.h>
#include <gcode/interp/Evaluator.h>

#include <cbang/Exception.h>
#include <cbang/SmartPointer.h>

#include <ostream>

namespace GCode {
  class Entity {
  protected:
    cb::LocationRange location;

  public:
    Entity() {}
    Entity(const Token &token) : location(token.getLocation()) {}
    virtual ~Entity() {}

    cb::LocationRange &getLocation() {return location;}
    const cb::LocationRange &getLocation() const {return location;}
    const int getLine() const {return location.getStart().getLine();}
    const int getCol() const {return location.getStart().getCol();}

    virtual bool isConstant() {return false;}
    virtual double eval(Evaluator &evaluator) {return 0;}
    virtual void print(std::ostream &stream) const = 0;

    template <typename T> T *instance() {return dynamic_cast<T *>(this);}
  };


  inline static
  std::ostream &operator<<(std::ostream &stream, const Entity &e) {
    e.print(stream);
    return stream;
  }
}
