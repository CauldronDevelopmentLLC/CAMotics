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
#include "python/PyLogger.h"
#include "python/Catch.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>
#include <cbang/util/Version.h>

#ifdef HAVE_EPOLL // Python.h defines this
#undef HAVE_EPOLL
#endif
#include <cbang/config.h>

#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
#endif


namespace CAMotics {
  namespace BuildInfo {
    void addBuildInfo(const char *category);
  }
}


PyObject *_set_logger(PyObject *mod, PyObject *args, PyObject *kwds) {
  try {
    const char *kwlist[] = {"callback", "level", "domainsLevels", 0};
    PyObject *cb;
    int level = -1;
    const char *domainLevels = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|is", (char **)kwlist,
                                     &cb, &level, &domainLevels)) return 0;

    if (cb == Py_None) cb::Logger::instance().setScreenStream(0);
    else cb::Logger::instance().setScreenStream(new PyLoggerStream(cb));

    if (0 <= level) cb::Logger::instance().setVerbosity(level);
    if (domainLevels) cb::Logger::instance().setLogDomainLevels(domainLevels);

    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyMethodDef _methods[] = {
  {"set_logger", (PyCFunction)_set_logger, METH_VARARGS | METH_KEYWORDS,
   "Set logger callback"},
  {0, 0, 0, 0}
};



static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "camotics",
  "CAMotics CNC simulator",
  -1,
  _methods,
  0, 0, 0, 0
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
  cb::Logger::instance().setLogColor(false);

#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 7
  if (!PyEval_ThreadsInitialized()) PyEval_InitThreads();
#endif

  PyObject *mod = PyModule_Create(&module);
  if (!mod) return 0;

  addType(mod, "Planner",    &PlannerType);
  addType(mod, "Simulation", &SimulationType);

  CAMotics::BuildInfo::addBuildInfo("CAMotics");
  cb::Version version(cb::Info::instance().get("CAMotics", "Version"));
  PyObject *v = Py_BuildValue("(iii)", version[0], version[1], version[2]);
  PyModule_AddObject(mod, "VERSION", v);

  return mod;
}
