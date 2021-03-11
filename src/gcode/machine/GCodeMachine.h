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


#include "MachineAdapter.h"

#include <gcode/Units.h>

#include <ostream>


namespace GCode {
  class GCodeMachine : public MachineAdapter {
    cb::SmartPointer<std::ostream> stream;
    Axes position;
    Units units;
    cb::FileLocation location;
    int axisSeen = 0;

  public:
    GCodeMachine(const cb::SmartPointer<std::ostream> &stream, Units units) :
      stream(stream), units(units) {}

    void beginLine();

    // From MachineInterface
    void start();
    void end();

    void setFeed(double feed);
    void setFeedMode(feed_mode_t mode);
    void setSpeed(double speed);
    void setSpinMode(spin_mode_t mode, double max);
    void setPathMode(path_mode_t mode, double motionBlending, double naiveCAM);
    void changeTool(unsigned tool);

    void input(port_t port, input_mode_t mode, double timeout);
    void seek(port_t port, bool active, bool error);
    void output(port_t port, double value);

    void setPosition(const Axes &position);

    void dwell(double seconds);
    void move(const Axes &position, int axes, bool rapid, double time);
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane);
    void pause(pause_t pause);

    void comment(const std::string &s) const;
    void message(const std::string &s);
  };
}
