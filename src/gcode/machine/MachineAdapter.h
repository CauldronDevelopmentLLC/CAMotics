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

#include <cbang/SmartPointer.h>


namespace GCode {
  class MachineAdapter : public MachineInterface {
    cb::SmartPointer<MachineInterface> parent;

  public:
    MachineAdapter(const cb::SmartPointer<MachineInterface> &parent = 0) :
      parent(parent) {}

    void setParent(const cb::SmartPointer<MachineInterface> &parent)
    {this->parent = parent;}
    const cb::SmartPointer<MachineInterface> &getParent() const {return parent;}

    template <typename T>
    T &find() {
      T *ptr = dynamic_cast<T *>(this);
      if (ptr) return *ptr;

      MachineAdapter *adapter = dynamic_cast<MachineAdapter *>(parent.get());
      if (!adapter) THROW("Not found");

      return adapter->find<T>();
    }

    // From MachineInterface
    void start() {parent->start();}
    void end() {parent->end();}

    double getFeed() const {return parent->getFeed();}
    void setFeed(double feed) {parent->setFeed(feed);}
    feed_mode_t getFeedMode() const {return parent->getFeedMode();}
    void setFeedMode(feed_mode_t mode){parent->setFeedMode(mode);}

    double getSpeed() const {return parent->getSpeed();}
    void setSpeed(double speed) {parent->setSpeed(speed);}

    spin_mode_t getSpinMode(double *max = 0) const
    {return parent->getSpinMode(max);}
    void setSpinMode(spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                     double max = 0) {parent->setSpinMode(mode, max);}

    void setPathMode(path_mode_t mode, double motionBlending = 0,
                     double naiveCAM = 0)
    {parent->setPathMode(mode, motionBlending, naiveCAM);}

    void changeTool(unsigned tool) {parent->changeTool(tool);}

    void input(port_t port, input_mode_t mode, double timeout)
    {parent->input(port, mode, timeout);}
    void seek(port_t port, bool active, bool error)
    {parent->seek(port, active, error);}
    void output(port_t port, double value) {parent->output(port, value);}

    Axes getPosition() const {return parent->getPosition();}
    cb::Vector3D getPosition(axes_t axes) const
    {return parent->getPosition(axes);}
    void setPosition(const Axes &position) {parent->setPosition(position);}

    void dwell(double seconds) {parent->dwell(seconds);}
    void move(const Axes &position, int axes, bool rapid)
    {parent->move(position, axes, rapid);}
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane)
    {parent->arc(offset, target, angle, plane);}

    Transforms &getTransforms() {return parent->getTransforms();}

    void pause(pause_t type) {parent->pause(type);}

    double get(address_t addr, Units units) const
    {return parent->get(addr, units);}
    void set(address_t addr, double value, Units units)
    {parent->set(addr, value, units);}

    bool has(const std::string &name) const {return parent->has(name);}
    double get(const std::string &name, Units units) const
    {return parent->get(name, units);}
    void set(const std::string &name, double value, Units units)
    {parent->set(name, value, units);}
    void clear(const std::string &name) {parent->clear(name);}

    const cb::LocationRange &getLocation() const {return parent->getLocation();}
    void setLocation(const cb::LocationRange &location)
    {parent->setLocation(location);}

    void comment(const std::string &s) const {parent->comment(s);}
    void message(const std::string &s) {parent->message(s);}
  };
}
