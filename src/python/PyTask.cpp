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

#include "PyTask.h"
#include "SmartPyGIL.h"

#include <cbang/Exception.h>

using namespace cb;
using namespace std;


void PyTask::join() {
  Py_BEGIN_ALLOW_THREADS;
  interrupt();
  Thread::join();
  Py_END_ALLOW_THREADS;
}


void PyTask::wait() {
  Py_BEGIN_ALLOW_THREADS;
  Thread::wait();
  Py_END_ALLOW_THREADS;
}


void PyTask::setCallback(PyObject *cb) {
  if (cb && cb != Py_None && !PyCallable_Check(cb))
    THROW("`callback` object not callable");

  this->cb = cb;
}


void PyTask::updated(const string &status, double progress) {
  if (!cb) return;

  SmartPyGIL gil;

  // Args
  PyObject *args = PyTuple_New(2);
  if (!args) {interrupt(); THROW("Failed to allocate tuple");}

  PyTuple_SetItem(args, 0, PyUnicode_FromString(status.c_str()));
  PyTuple_SetItem(args, 1, PyFloat_FromDouble(progress));

  // Call
  PyObject *result = PyObject_Call(cb.get(), args, 0);
  Py_DECREF(args);

  // Check result
  if (!result) {interrupt(); THROW("Simulation callback failed");}
  if (result != Py_None && !PyObject_IsTrue(result)) interrupt();
  Py_DECREF(result);

  // Check if we should stop
  if (shouldShutdown()) interrupt();
}
