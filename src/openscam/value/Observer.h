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

#ifndef OPENSCAM_OBSERVER_H
#define OPENSCAM_OBSERVER_H

#include <cbang/String.h>
#include <cbang/StdTypes.h>


namespace OpenSCAM {
  class Observer {
  public:
    virtual ~Observer() {} // Compiler needs this

    virtual void updated(const std::string &name, const std::string &value) = 0;
    virtual void updated(const std::string &name, const char *value)
    {updated(name, std::string(value));}

    virtual void updated(const std::string &name, int64_t value)
    {updated(name, cb::String(value));}
    virtual void updated(const std::string &name, uint64_t value)
    {updated(name, (int64_t)value);}
    virtual void updated(const std::string &name, int32_t value)
    {updated(name, (int64_t)value);}
    virtual void updated(const std::string &name, uint32_t value)
    {updated(name, (int64_t)value);}
    virtual void updated(const std::string &name, int16_t value)
    {updated(name, (int64_t)value);}
    virtual void updated(const std::string &name, uint16_t value)
    {updated(name, (int64_t)value);}

    virtual void updated(const std::string &name, double value)
    {updated(name, cb::String(value));}
    virtual void updated(const std::string &name, float value)
    {updated(name, (double)value);}

    virtual void updated(const std::string &name, bool value)
    {updated(name, cb::String(value));}
  };
}

#endif // OPENSCAM_OBSERVER_H

