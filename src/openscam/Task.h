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

#ifndef OPENSCAM_TASK_H
#define OPENSCAM_TASK_H

#include <cbang/os/Mutex.h>

namespace OpenSCAM {
  class Task : public cb::Mutex {
    bool interrupted;

    double startTime;
    double endTime;
    std::string status;
    double progress;
    double eta;

  public:
    Task() :
      interrupted(false), startTime(0), endTime(0), status("Idle"), progress(0),
      eta(0) {}
    virtual ~Task() {}

    virtual void interrupt() {interrupted = true;}
    virtual bool shouldQuit() const {return interrupted;}
    virtual const std::string &getStatus() const;
    virtual double getProgress() const;
    virtual double getETA() const;
    virtual double getTime() const;

    void begin();
    void update(double progress, const std::string &status = std::string());
    double end();
  };
}

#endif // OPENSCAM_TASK_H

