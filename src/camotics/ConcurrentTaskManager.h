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


#include "Task.h"
#include "TaskObserver.h"

#include <cbang/SmartPointer.h>
#include <cbang/os/Thread.h>
#include <cbang/os/Condition.h>

#include <list>
#include <set>


namespace CAMotics {
  class ConcurrentTaskManager : public cb::Thread, public cb::Condition {
    typedef std::list<cb::SmartPointer<Task> > queue_t;

    queue_t waiting;
    cb::SmartPointer<Task> current;
    queue_t done;

    typedef std::set<TaskObserver *> observers_t;
    observers_t observers;

  public:
    ConcurrentTaskManager();
    ~ConcurrentTaskManager();

    double getProgress() const;
    double getETA() const;
    std::string getStatus() const;

    void addTask(const cb::SmartPointer<Task> &task, bool priority = true);
    bool hasMore() const;
    cb::SmartPointer<Task> remove();
    void addObserver(TaskObserver *observer);
    void interrupt();

    // From Thread
    void run();
    void stop();

  protected:
    void interruptTasks();
    void complete(const cb::SmartPointer<Task> &task);
  };
}
