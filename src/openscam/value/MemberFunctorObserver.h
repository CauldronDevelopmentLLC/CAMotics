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

#ifndef OPENSCAM_MEMBER_FUNCTOR_OBSERVER_H
#define OPENSCAM_MEMBER_FUNCTOR_OBSERVER_H

#include "Observer.h"


namespace OpenSCAM {
  template <typename CLASS, typename T>
  class MemberFunctorObserver : public Observer {
    CLASS *object;
    typedef void (CLASS::*member_t)(const std::string &, T);
    member_t member;

  public:
    MemberFunctorObserver(CLASS *object, member_t member) :
      object(object), member(member) {}

    // From Observer
    using Observer::updated;
    void updated(const std::string &name, const std::string &value) {}
    void updated(const std::string &name, T value)
    {(*object.*member)(name, value);}
  };
}

#endif // OPENSCAM_MEMBER_FUNCTOR_OBSERVER_H

