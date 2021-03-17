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

#include <cbang/Catch.h>
#include <cbang/SmartPointer.h>


namespace GCode {
  class MachineAdapter : public MachineNode, public MachineInterface {
    class _ {
      const MachineAdapter &adapter;
    public:
      _(const MachineAdapter *adapter) : adapter(*adapter) {
        CBANG_TRY_CATCH_ERROR(adapter->enter());
      }
      ~_() {CBANG_TRY_CATCH_ERROR(adapter.exit());}
    };

  public:
    MachineAdapter(const cb::SmartPointer<MachineInterface> &next = 0) :
      MachineNode(next) {}

    virtual void enter() const {}
    virtual void exit() const {}

    // From MachineInterface
    void start() {_ _(this); next->start();}
    void end() {_ _(this); next->end();}

    double getFeed() const {_ _(this); return next->getFeed();}
    void setFeed(double feed) {_ _(this); next->setFeed(feed);}
    feed_mode_t getFeedMode() const {_ _(this); return next->getFeedMode();}
    void setFeedMode(feed_mode_t mode){_ _(this); next->setFeedMode(mode);}

    double getSpeed() const {_ _(this); return next->getSpeed();}
    void setSpeed(double speed) {_ _(this); next->setSpeed(speed);}

    spin_mode_t getSpinMode(double *max = 0) const
    {_ _(this); return next->getSpinMode(max);}
    void setSpinMode(spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                     double max = 0) {_ _(this); next->setSpinMode(mode, max);}

    void setPathMode(path_mode_t mode, double motionBlending = 0,
                     double naiveCAM = 0)
    {_ _(this); next->setPathMode(mode, motionBlending, naiveCAM);}

    void changeTool(unsigned tool) {_ _(this); next->changeTool(tool);}

    void input(port_t port, input_mode_t mode, double timeout)
    {_ _(this); next->input(port, mode, timeout);}
    void seek(port_t port, bool active, bool error)
    {_ _(this); next->seek(port, active, error);}
    void output(port_t port, double value)
    {_ _(this); next->output(port, value);}

    Axes getPosition() const {_ _(this); return next->getPosition();}
    cb::Vector3D getPosition(axes_t axes) const
    {_ _(this); return next->getPosition(axes);}
    void setPosition(const Axes &position)
    {_ _(this); next->setPosition(position);}

    void dwell(double seconds) {_ _(this); next->dwell(seconds);}
    void move(const Axes &position, int axes, bool rapid, double time)
    {_ _(this); next->move(position, axes, rapid, time);}
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane)
    {_ _(this); next->arc(offset, target, angle, plane);}

    Transforms &getTransforms() {_ _(this); return next->getTransforms();}

    void pause(pause_t type) {_ _(this); next->pause(type);}

    double get(address_t addr, Units units) const
    {_ _(this); return next->get(addr, units);}
    void set(address_t addr, double value, Units units)
    {_ _(this); next->set(addr, value, units);}

    bool has(const std::string &name) const {_ _(this); return next->has(name);}
    double get(const std::string &name, Units units) const
    {_ _(this); return next->get(name, units);}
    void set(const std::string &name, double value, Units units)
    {_ _(this); next->set(name, value, units);}
    void clear(const std::string &name) {_ _(this); next->clear(name);}

    const cb::LocationRange &getLocation() const
    {_ _(this); return next->getLocation();}
    void setLocation(const cb::LocationRange &location)
    {_ _(this); next->setLocation(location);}

    void comment(const std::string &s) const {_ _(this); next->comment(s);}
    void message(const std::string &s) {_ _(this); next->message(s);}
  };
}
