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


#include "MachineInterface.h"

#include <gcode/Addresses.h>
#include <gcode/Parameter.h>

#include <map>


namespace GCode {
  class MachineState : public MachineInterface {
    bool started;

    double feed;
    feed_mode_t feedMode;

    double speed;
    spin_mode_t spinMode;
    double maxSpeed;

    Axes position;

    Transforms transforms;

    // Numbered and named parameters
    Parameter params[MAX_ADDRESS];
    std::map<std::string, Parameter> named;

    cb::LocationRange location;

  public:
    MachineState();

    // From MachineInterface
    void start();
    void end();

    double getFeed() const {return feed;}
    void setFeed(double feed);
    feed_mode_t getFeedMode() const {return feedMode;}
    void setFeedMode(feed_mode_t feedMode){this->feedMode = feedMode;}

    double getSpeed() const {return speed;}
    void setSpeed(double speed);
    spin_mode_t getSpinMode(double *max = 0) const
    {if (max) *max = maxSpeed; return spinMode;}
    void setSpinMode(spin_mode_t mode, double max)
    {spinMode = mode; maxSpeed = max;}
    void setPathMode(path_mode_t mode, double motionBlending, double naiveCAM);

    void changeTool(unsigned tool);

    void input(port_t port, input_mode_t mode, double timeout) {}
    void seek(port_t port, bool active, bool error) {}
    void output(port_t port, double value) {}

    Axes getPosition() const {return position;}
    cb::Vector3D getPosition(axes_t axes) const;
    void setPosition(const Axes &position);

    void dwell(double seconds) {}
    void move(const Axes &position, int axes, bool rapid, double time)
    {setPosition(position);}
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane) {position.setXYZ(target);}

    Transforms &getTransforms() {return transforms;}

    void pause(pause_t type) {}

    double get(address_t addr, Units units) const;
    void set(address_t addr, double value, Units units);

    bool has(const std::string &name) const;
    double get(const std::string &name, Units units) const;
    void set(const std::string &name, double value, Units units);
    void clear(const std::string &name);

    const cb::LocationRange &getLocation() const {return location;}
    void setLocation(const cb::LocationRange &location)
    {this->location = location;}

    void comment(const std::string &s) const {}
    void message(const std::string &s) {}
  };
}
