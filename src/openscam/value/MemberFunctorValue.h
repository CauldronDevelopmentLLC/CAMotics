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

#ifndef OPENSCAM_MEMBER_FUNCTOR_VALUE_H
#define OPENSCAM_MEMBER_FUNCTOR_VALUE_H

#include "Value.h"


namespace OpenSCAM {
  template <typename CLASS, typename T>
  class MemberFunctorValue : public Value {
    CLASS *object;
    typedef T (CLASS::*member_t)() const;
    member_t member;

  public:
    MemberFunctorValue(const std::string name, CLASS *object, member_t member) :
      Value(name), object(object), member(member) {}

    // From Value
    void updated() {set((*object.*member)());}
  };
}

#endif // OPENSCAM_MEMBER_FUNCTOR_VALUE_H

