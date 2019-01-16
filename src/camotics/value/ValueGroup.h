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


#include "ValueSet.h"

#include <vector>


namespace CAMotics {
  class ValueGroup : public std::vector<cb::SmartPointer<Value> > {
    ValueSet &set;

  public:
    ValueGroup(ValueSet &set) : set(set) {}

    cb::SmartPointer<Value>
    add(const cb::SmartPointer<Value> &value) {return set.add(value);}

    template <typename T> cb::SmartPointer<Value>
    add(const std::string &name, T &var) {
      const cb::SmartPointer<Value> &value = set.add(name, var);
      push_back(value);
      return value;
    }


    template <typename CLASS, typename T> cb::SmartPointer<Value>
    add(const std::string &name, CLASS *object, T (CLASS::*member)() const) {
      const cb::SmartPointer<Value> &value =
        set.add(name, object, member);
      push_back(value);
      return value;
    }

    void updated();
  };
}
