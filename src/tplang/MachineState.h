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

#ifndef TPLANG_MACHINE_STATE_H
#define TPLANG_MACHINE_STATE_H

#include "MachineInterface.h"


namespace tplang {
  class MachineState : public MachineInterface {
    bool started;

    double feed;
    feed_mode_t feedMode;

    double speed;
    spin_mode_t spinMode;
    double maxSpeed;

    unsigned tool;

    Axes position;

    cb::Matrix4x4D matrices[AXES_COUNT];

    cb::LocationRange location;

  public:
    MachineState() {reset();}

    // From MachineInterface
    void reset();
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

    unsigned getTool() const {return tool;}
    void setTool(unsigned tool) {this->tool = tool;}

    int findPort(port_t type, unsigned index) {return -1;}
    double input(unsigned port, input_mode_t mode, double timeout, bool error)
    {return 0;}
    void output(unsigned port, double value, bool sync) {}

    Axes getPosition() const {return position;}
    cb::Vector3D getPosition(axes_t axes) const;

    void dwell(double seconds) {}
    void move(const Axes &axes, bool rapid) {position = axes;}
    void arc(const cb::Vector3D &offset, double angle, plane_t plane) {}

    const cb::Matrix4x4D &getMatrix(axes_t matrix) const;
    void setMatrix(const cb::Matrix4x4D &m, axes_t matrix);

    void pause(bool optional) const {}
    bool synchronize(double timeout) const {return true;}
    void abort() {}

    async_error_t readAsyncError() {return OK;}
    void clearAsyncErrors() {}

    const cb::LocationRange &getLocation() const {return location;}
    void setLocation(const cb::LocationRange &location)
    {this->location = location;}
  };
}

#endif // TPLANG_MACHINE_STATE_H

