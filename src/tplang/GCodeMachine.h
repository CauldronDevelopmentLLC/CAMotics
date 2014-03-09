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

#ifndef TPLANG_GCODE_MACHINE_H
#define TPLANG_GCODE_MACHINE_H

#include "MachineAdapter.h"

#include <ostream>

namespace tplang {
  class GCodeMachine : public MachineAdapter {
    std::ostream &stream;

    bool mistCoolant;
    bool floodCoolant;

    Axes position;

  public:
    GCodeMachine(std::ostream &stream) :
      stream(stream), mistCoolant(false), floodCoolant(false) {}

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
  };
}

#endif // TPLANG_GCODE_MACHINE_H
