/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "OffsetType.h"

#include <string>


namespace CAMotics {
  class CAMLayer : public OffsetType {
  public:
    std::string name;
    unsigned tool;
    unsigned feed;
    unsigned speed;
    OffsetType offsetType;
    double offset;
    double startDepth;
    double endDepth;
    double maxStep;

    CAMLayer(const std::string &name, unsigned tool, unsigned feed,
             unsigned speed, OffsetType offsetType, double offset,
             double startDepth, double endDepth, double maxStep) :
      name(name), tool(tool), feed(feed), speed(speed), offsetType(offsetType),
      offset(offset), startDepth(startDepth), endDepth(endDepth),
      maxStep(maxStep) {}

    std::string getOffsetString() const;
  };
}
