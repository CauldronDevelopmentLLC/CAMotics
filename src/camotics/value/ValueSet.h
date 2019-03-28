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


#include "Value.h"
#include "VarValue.h"
#include "MemberFunctorValue.h"

#include <map>

namespace CAMotics {
  class ValueSet {
    typedef std::map<std::string, cb::SmartPointer<Value> >
    values_t;
    values_t values;

  public:
    typedef values_t::const_iterator iterator;
    iterator begin() const {return values.begin();}
    iterator end() const {return values.end();}

    iterator find(const std::string &name) const {return values.find(name);}

    const cb::SmartPointer<Value> &get(const std::string &name) const;
    const cb::SmartPointer<Value> &operator[](const std::string &name) const
    {return get(name);}

    cb::SmartPointer<Value> add(const cb::SmartPointer<Value> &value) {
      iterator it = find(value->getName());

      if (it != end())
        THROW("Value with name '" << value->getName() << "' already in set");

      values.insert(values_t::value_type(value->getName(), value));

      return value;
    }


    template <typename T> cb::SmartPointer<Value>
    add(const std::string &name, T &var)
    {return add(new VarValue<T>(name, var));}

    template <typename CLASS, typename T> cb::SmartPointer<Value>
    add(const std::string &name, CLASS *object, T (CLASS::*member)() const)
    {return add(new MemberFunctorValue<CLASS, T>(name, object, member));}

    void updated();
  };
}
