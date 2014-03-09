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

#ifndef OPENSCAM_VALUE_H
#define OPENSCAM_VALUE_H

#include "Observer.h"
#include "MemberFunctorObserver.h"

#include <cbang/SmartPointer.h>

#include <vector>


namespace OpenSCAM {
  class Value {
    std::string name;

    typedef std::vector<cb::SmartPointer<Observer> > observers_t;
    observers_t observers;

  public:
    Value(const std::string &name) : name(name) {}
    virtual ~Value() {} // Complier needs this

    const std::string &getName() const {return name;}
    void add(const cb::SmartPointer<Observer> &o) {observers.push_back(o);}

    template <typename CLASS, typename T>
    void add(CLASS *object, void (CLASS::*member)(const std::string &, T))
    {add(new MemberFunctorObserver<CLASS, T>(object, member));}

    template <typename T>
    void set(const T &value) {
      for (unsigned i = 0; i < observers.size(); i++)
        observers[i]->updated(name, value);
    }

    virtual void updated() = 0;
  };
}

#endif // OPENSCAM_VALUE_H

