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

#include "python/PyPlanner.h"
#include "python/PySimulation.h"

#include <cbang/log/Logger.h>

#ifdef HAVE_EPOLL // Python.h defines this
#undef HAVE_EPOLL
#endif
#include <cbang/config.h>

#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
#endif


static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT, "camotics", 0, -1, 0, 0, 0, 0, 0
};


static void addType(PyObject *mod, const char *name, PyTypeObject *type) {
  if (0 <= PyType_Ready(type)) {
    Py_INCREF(type);
    PyModule_AddObject(mod, name, (PyObject *)type);
  }
}


PyMODINIT_FUNC PyInit_camotics() {
#ifdef HAVE_V8
  cb::gv8::JSImpl::init(0, 0);
#endif

  // Configure logger
  cb::Logger::instance().setLogTime(false);
  cb::Logger::instance().setLogShortLevel(true);
  cb::Logger::instance().setLogPrefix(true);

  if (!PyEval_ThreadsInitialized()) PyEval_InitThreads();

  PyObject *mod = PyModule_Create(&module);
  if (!mod) return 0;

  addType(mod, "Planner",    &PlannerType);
  addType(mod, "Simulation", &SimulationType);

  return mod;
}
