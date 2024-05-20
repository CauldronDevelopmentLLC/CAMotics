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
    void start() override;
    void end() override;

    double getFeed() const override {return feed;}
    void setFeed(double feed) override;
    feed_mode_t getFeedMode() const override {return feedMode;}
    void setFeedMode(feed_mode_t feedMode) override {this->feedMode = feedMode;}

    double getSpeed() const override {return speed;}
    void setSpeed(double speed) override;
    spin_mode_t getSpinMode(double *max = 0) const override
    {if (max) *max = maxSpeed; return spinMode;}
    void setSpinMode(spin_mode_t mode, double max) override
    {spinMode = mode; maxSpeed = max;}
    void setPathMode(path_mode_t mode, double motionBlending,
                     double naiveCAM) override;

    void changeTool(unsigned tool) override;

    void input(port_t port, input_mode_t mode, double timeout) override {}
    void seek(port_t port, bool active, bool error) override {}
    void output(port_t port, double value) override {}

    Axes getPosition() const override {return position;}
    cb::Vector3D getPosition(axes_t axes) const override;
    void setPosition(const Axes &position) override;

    void dwell(double seconds) override {}
    void move(const Axes &position, int axes, bool rapid, double time) override
    {setPosition(position);}
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane) override {position.setXYZ(target);}

    Transforms &getTransforms() override {return transforms;}

    void pause(pause_t type) override {}

    double get(address_t addr, Units units) const override;
    void set(address_t addr, double value, Units units) override;

    bool has(const std::string &name) const override;
    double get(const std::string &name, Units units) const override;
    void set(const std::string &name, double value, Units units) override;
    void clear(const std::string &name) override;

    const cb::LocationRange &getLocation() const override {return location;}
    void setLocation(const cb::LocationRange &location) override
    {this->location = location;}

    void comment(const std::string &s) const override {}
    void message(const std::string &s) override {}
  };
}
