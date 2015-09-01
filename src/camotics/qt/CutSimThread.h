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

#ifndef CAMOTICS_CUT_SIM_THREAD_H
#define CAMOTICS_CUT_SIM_THREAD_H

#include "BackgroundThread.h"

#include <camotics/cutsim/CutSim.h>

#include <cbang/SmartPointer.h>
#include <cbang/os/Thread.h>

#include <QWidget>


namespace CAMotics {
  class CutSim;

  class CutSimThread : public BackgroundThread {
  protected:
    cb::SmartPointer<CutSim> cutSim;

  public:
    CutSimThread(cb::Application &app, int event, QWidget *parent,
                 const cb::SmartPointer<CutSim> &cutSim) :
      BackgroundThread(app, event, parent), cutSim(cutSim) {}

    unsigned getErrorCount() const {return cutSim->getErrorCount();}

    // From cb::Thread
    void stop();
  };
}

#endif // CAMOTICS_CUT_SIM_THREAD_H

