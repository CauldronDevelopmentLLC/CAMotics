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

#ifndef TPLANG_MACHINE_ADAPTER_H
#define TPLANG_MACHINE_ADAPTER_H

#include "MachineInterface.h"

#include <cbang/SmartPointer.h>


namespace tplang {
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
    void reset() {parent->reset();}
    void start() {parent->start();}
    void end() {parent->end();}

    double getFeed(feed_mode_t *mode = 0) const {return parent->getFeed(mode);}
    void setFeed(double feed, feed_mode_t mode = MM_PER_MINUTE)
    {parent->setFeed(feed, mode);}

    double getSpeed(spin_mode_t *mode = 0, double *max = 0) const
    {return parent->getSpeed(mode, max);}
    void setSpeed(double speed, spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                  double max = 0) {parent->setSpeed(speed, mode, max);}

    unsigned getTool() const {return parent->getTool();}
    void setTool(unsigned tool) {parent->setTool(tool);}

    int findPort(port_t type, unsigned index = 0)
    {return parent->findPort(type, index);}
    double input(unsigned port, input_mode_t mode = IMMEDIATE,
                 double timeout = 0, bool error = false)
    {return parent->input(port, mode, timeout, error);}
    void output(unsigned port, double value, bool sync = true)
    {parent->output(port, value, sync);}

    Axes getPosition() const {return parent->getPosition();}
    cb::Vector3D getPosition(axes_t axes) const
    {return parent->getPosition(axes);}

    void dwell(double seconds) {parent->dwell(seconds);}
    void move(const Axes &axes, bool rapid = false) {parent->move(axes, rapid);}
    void arc(const cb::Vector3D &offset, double angle, plane_t plane = XY)
    {parent->arc(offset, angle, plane);}

    const cb::Matrix4x4D &getMatrix(axes_t matrix) const
    {return parent->getMatrix(matrix);}
    void setMatrix(const cb::Matrix4x4D &m, axes_t matrix)
    {parent->setMatrix(m, matrix);}

    void pause(bool optional = true) const {parent->pause(optional);}
    bool synchronize(double timeout = 0) const
    {return parent->synchronize(timeout);}
    void abort() {parent->abort();}

    async_error_t readAsyncError() {return parent->readAsyncError();}
    void clearAsyncErrors() {parent->clearAsyncErrors();}

    const cb::LocationRange &getLocation() const {return parent->getLocation();}
    void setLocation(const cb::LocationRange &location)
    {parent->setLocation(location);}
  };
}

#endif // TPLANG_MACHINE_ADAPTER_H

