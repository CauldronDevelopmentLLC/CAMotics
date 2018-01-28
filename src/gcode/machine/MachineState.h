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


#include "MachineInterface.h"

#include <gcode/Addresses.h>

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

    cb::Matrix4x4D matrices[AXES_COUNT];

    // Numbered and named parameters
    double params[MAX_ADDRESS];
    typedef std::map<std::string, double> named_t;
    named_t named;

    cb::LocationRange location;

  public:
    MachineState();

    // From MachineInterface
    void start();
    void end();

    double getFeed(feed_mode_t *mode = 0) const
    {if (mode) *mode = feedMode; return feed;}
    void setFeed(double feed, feed_mode_t mode)
    {this->feed = feed; feedMode = mode;}

    double getSpeed(spin_mode_t *mode = 0, double *max = 0) const
    {if (mode) *mode = spinMode; if (max) *max = maxSpeed; return speed;}
    void setSpeed(double speed, spin_mode_t mode, double max)
    {this->speed = speed; spinMode = mode; maxSpeed = max;}

    void changeTool(unsigned tool);

    void wait(port_t port, bool active, double timeout) {}
    void seek(port_t port, bool active, bool error) {}
    void output(port_t port, double value) {}

    Axes getPosition() const {return position;}
    cb::Vector3D getPosition(axes_t axes) const;

    void dwell(double seconds) {}
    void move(const Axes &axes, bool rapid) {position = axes;}
    void arc(const cb::Vector3D &offset, double angle, plane_t plane) {}

    const cb::Matrix4x4D &getMatrix(axes_t matrix) const;
    void setMatrix(const cb::Matrix4x4D &m, axes_t matrix);

    void pause(bool optional) {}

    double get(address_t addr) const;
    void set(address_t addr, double value);

    bool has(const std::string &name) const;
    double get(const std::string &name) const;
    void set(const std::string &name, double value);

    const cb::LocationRange &getLocation() const {return location;}
    void setLocation(const cb::LocationRange &location)
    {this->location = location;}

    void comment(const std::string &s) const {}
  };
}
