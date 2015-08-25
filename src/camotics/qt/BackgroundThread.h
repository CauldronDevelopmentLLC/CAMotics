/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_BACKGROUND_THREAD_H
#define CAMOTICS_BACKGROUND_THREAD_H

#include <cbang/SmartPointer.h>
#include <cbang/os/Thread.h>

#include <QWidget>

namespace cb {class Application;}


namespace CAMotics {
  class BackgroundThread : public cb::Thread {
    cb::Application &app;
    int event;
    QWidget *parent;

  public:
    BackgroundThread(cb::Application &app, int event, QWidget *parent) :
      app(app), event(event), parent(parent) {}
    ~BackgroundThread() {join();}

    void completed();
  };
}

#endif // CAMOTICS_BACKGROUND_THREAD_H

