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


#include "MachineAdapter.h"

#include <camotics/Units.h>

#include <ostream>

namespace CAMotics {
  class GCodeMachine : public MachineAdapter {
    std::ostream &stream;
    Units units;

    bool mistCoolant;
    bool floodCoolant;

    Axes position;

    cb::FileLocation location;

  public:
    GCodeMachine(std::ostream &stream, Units units) :
      stream(stream), units(units), mistCoolant(false), floodCoolant(false) {}

    void beginLine();

    // From MachineInterface
    void start();
    void end();

    void setFeed(double feed, feed_mode_t mode);
    void setSpeed(double speed, spin_mode_t mode, double max);
    void setTool(unsigned tool);

    int findPort(port_t type, unsigned index);
    double input(unsigned port, input_mode_t mode, double timeout, bool error);
    void output(unsigned port, double value, bool sync);

    void dwell(double seconds);
    void move(const Axes &axes, bool rapid);
    void pause(bool optional);
  };
}


