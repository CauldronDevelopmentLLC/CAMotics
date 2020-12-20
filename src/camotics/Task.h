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

#include <cbang/os/Condition.h>


namespace CAMotics {
  class Task : public cb::Condition {
    bool interrupted = false;

    double startTime = 0;
    double endTime = 0;
    std::string status = "Idle";
    double progress = 0;
    double eta = 0;

  public:
    Task() {}
    virtual ~Task() {}

    virtual void interrupt() {interrupted = true;}
    virtual bool shouldQuit() const {return interrupted;}
    virtual std::string getStatus() const;
    virtual double getProgress() const;
    virtual double getETA() const;
    virtual double getTime() const;

    virtual void updated(const std::string &status, double progress) {}

    void begin(const std::string &status);
    bool update(double progress);

    virtual void run() {};
  };
}
