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

#include "Task.h"

#include <cbang/util/SmartLock.h>
#include <cbang/time/Timer.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


const string &Task::getStatus() const {
  SmartLock lock(this);
  return status;
}


double Task::getProgress() const {
  SmartLock lock(this);
  return progress;
}


double Task::getETA() const {
  SmartLock lock(this);
  return eta;
}


double Task::getTime() const {
  SmartLock lock(this);
  return endTime - startTime;
}


void Task::begin() {
  SmartLock lock(this);
  interrupted = false;
  status.clear();
  progress = eta = 0;
  startTime = endTime = Timer::now();
}


void Task::update(double progress, const string &status) {
  SmartLock lock(this);

  this->progress = progress;
  this->status = status;

  endTime = Timer::now();
  double delta = endTime - startTime;
  if (progress && 1 < delta) eta = delta / progress - delta;
  else eta = 0;
}


double Task::end() {
  SmartLock lock(this);
  status = "Idle";
  endTime = Timer::now();
  return endTime - startTime;
}
