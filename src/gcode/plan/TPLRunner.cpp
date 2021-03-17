/******************************************************************************\

             CAMotics is an Open-Source simulation and CAM software.
     Copyright (C) 2011-2021 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "TPLRunner.h"

#include <gcode/machine/MachinePipeline.h>

#include <tplang/TPLContext.h>

#include <cbang/config.h>
#include <cbang/Catch.h>
#include <cbang/js/JSInterrupted.h>

#include <iostream>

using namespace GCode;
using namespace cb;
using namespace std;


TPLRunner::TPLRunner(MachinePipeline &pipeline, const cb::InputSource &source,
                     const PlannerConfig &config) :
  Runner(config),
  MachineAdapter(SmartPointer<MachineInterface>::Phony(&pipeline)),
  source(source) {

  Condition::lock();
  Thread::start();
}


TPLRunner::~TPLRunner() {
  if (ctx.isSet()) ctx->interrupt();
  Condition::unlock();
  Thread::join();
}


void TPLRunner::enter() const {
  Condition::lock();
  auto trace = ctx->getStackTrace(1);
  if (!trace->empty()) getNextNode()->setLocation(trace->at(0));
}


void TPLRunner::exit() const {
  Condition::signal();
  Condition::unlock();
}


bool TPLRunner::hasMore() const {return Thread::isRunning();}


void TPLRunner::step() {
  if (done) Thread::join();
  else Condition::wait();
}


void TPLRunner::run() {
  try {
    Condition::lock();
    ctx = new tplang::TPLContext(SmartPointer<ostream>::Phony(&cerr), *this);
    Condition::unlock();

    try {
      ctx->eval(source);
    } catch (const js::JSInterrupted &e) {}
  } CATCH_ERROR;

  Condition::lock();
  ctx.release();
  done = true;
  Condition::signal();
  Condition::unlock();
}
