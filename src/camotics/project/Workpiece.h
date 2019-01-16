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

#include <cbang/geom/Rectangle.h>
#include <cbang/json/Serializable.h>

#include <string>


namespace GCode {class ToolPath;}

namespace CAMotics {
  namespace Project {
    class Workpiece : public cb::JSON::Serializable {
      bool automatic;
      double margin;
      cb::Rectangle3D bounds;

    public:
      Workpiece() : automatic(true), margin(5) {}

      bool isAutomatic() const {return automatic;}
      void setAutomatic(bool x) {automatic = x;}

      double getMargin() const {return margin;}
      void setMargin(double x) {margin = x;}

      void setBounds(const cb::Rectangle3D &bounds);
      cb::Rectangle3D getBounds() const {return bounds;}

      void update(GCode::ToolPath &path);

      // From JSON::Serializable
      void read(const cb::JSON::Value &value);
      void write(cb::JSON::Sink &sink) const;
    };
  }
}
