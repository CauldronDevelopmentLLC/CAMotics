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


#include "MachineNode.h"

#include <cbang/SmartPointer.h>


namespace GCode {
  class MachineAdapter : public MachineNode, public MachineInterface {
  public:
    MachineAdapter(const cb::SmartPointer<MachineInterface> &next = 0) :
      MachineNode(next) {}

    // From MachineInterface
    void start() {next->start();}
    void end() {next->end();}

    double getFeed() const {return next->getFeed();}
    void setFeed(double feed) {next->setFeed(feed);}
    feed_mode_t getFeedMode() const {return next->getFeedMode();}
    void setFeedMode(feed_mode_t mode){next->setFeedMode(mode);}

    double getSpeed() const {return next->getSpeed();}
    void setSpeed(double speed) {next->setSpeed(speed);}

    spin_mode_t getSpinMode(double *max = 0) const
    {return next->getSpinMode(max);}
    void setSpinMode(spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                     double max = 0) {next->setSpinMode(mode, max);}

    void setPathMode(path_mode_t mode, double motionBlending = 0,
                     double naiveCAM = 0)
    {next->setPathMode(mode, motionBlending, naiveCAM);}

    void changeTool(unsigned tool) {next->changeTool(tool);}

    void input(port_t port, input_mode_t mode, double timeout)
    {next->input(port, mode, timeout);}
    void seek(port_t port, bool active, bool error)
    {next->seek(port, active, error);}
    void output(port_t port, double value) {next->output(port, value);}

    Axes getPosition() const {return next->getPosition();}
    cb::Vector3D getPosition(axes_t axes) const
    {return next->getPosition(axes);}
    void setPosition(const Axes &position) {next->setPosition(position);}

    void dwell(double seconds) {next->dwell(seconds);}
    void move(const Axes &position, int axes, bool rapid, double time)
    {next->move(position, axes, rapid, time);}
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane)
    {next->arc(offset, target, angle, plane);}

    Transforms &getTransforms() {return next->getTransforms();}

    void pause(pause_t type) {next->pause(type);}

    double get(address_t addr, Units units) const
    {return next->get(addr, units);}
    void set(address_t addr, double value, Units units)
    {next->set(addr, value, units);}

    bool has(const std::string &name) const {return next->has(name);}
    double get(const std::string &name, Units units) const
    {return next->get(name, units);}
    void set(const std::string &name, double value, Units units)
    {next->set(name, value, units);}
    void clear(const std::string &name) {next->clear(name);}

    const cb::LocationRange &getLocation() const {return next->getLocation();}
    void setLocation(const cb::LocationRange &location)
    {next->setLocation(location);}

    void comment(const std::string &s) const {next->comment(s);}
    void message(const std::string &s) {next->message(s);}
  };
}
