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

#include "PySimulation.h"
#include "PyJSON.h"
#include "PyJSONSink.h"
#include "PyLogger.h"
#include "PyTask.h"
#include "SmartPyGIL.h"
#include "Catch.h"

#include <camotics/sim/Simulation.h>
#include <camotics/sim/SimulationRun.h>
#include <camotics/sim/ToolPathTask.h>
#include <camotics/project/Project.h>
#include <camotics/contour/Surface.h>

#include <cbang/Catch.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/os/SystemInfo.h>

#include <limits>

using namespace CAMotics;
using namespace cb;
using namespace std;


namespace {
  struct Simulation {
    Project::Project project;
    SmartPointer<GCode::ToolPath> path;
    SmartPointer<Surface> surface;
    SmartPointer<PyTask> task;
  };


  typedef struct {
    PyObject_HEAD;
    Simulation *s;
  } PySimulation;


  void _dealloc(PySimulation *self) {
    if (self->s) {
      // End task gracefully
      if (self->s->task.isSet()) self->s->task->join();
      delete self->s;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
  }


  PyObject *_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return type->tp_alloc(type, 0);
  }


  int _init(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      self->s = new Simulation;
      return 0;
    } CATCH_PYTHON;

    return -1;
  }


  void set_task(PySimulation *self, const SmartPointer<PyTask> &task) {
    if (self->s->task.isSet()) THROW("A task is already active");
    self->s->task = task;
  }


  PyObject *_open(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"filename", 0};
      const char *filename = 0;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist,
                                       &filename))
        return 0;

      if (!filename) {
        PyErr_SetString(PyExc_RuntimeError, "Missing filename.");
        return 0;
      }

      string ext = SystemUtilities::extension(filename);
      if (ext == "xml" || ext == "camotics") s.project.load(filename);
      else s.project.addFile(filename); // Assume TPL or G-Code

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_resolution(PySimulation *self, PyObject *args,
                            PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"resolution", "mode", 0};
      double resolution = 0;
      const char *mode = 0;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "ds", (char **)kwlist,
                                       &resolution, &mode))
        return 0;

      if (mode) s.project.setResolutionMode(ResolutionMode::parse(mode));
      if (resolution) s.project.setResolution(resolution);

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_compute_path(PySimulation *self) {
    try {
      class Runner : public PyTask {
        Simulation &s;
        ToolPathTask task;

      public:
        Runner(PySimulation *self) : s(*self->s), task(s.project) {start();}

        // From Thread
        void run() {
          task.run();
          SmartPyGIL gil;
          s.path = task.getPath();
        }
      };

      set_task(self, new Runner(self));

      Py_RETURN_NONE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_path(PySimulation *self) {
    try {
      if (!self->s->path.isSet()) THROW("Tool path not set");

      PyJSONSink sink;
      self->s->path->write(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_start(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      class Runner : public PyTask {
        Simulation &s;
        double time = 0;
        unsigned threads = 0;
        int reduce = false;

        SmartPointer<GCode::ToolPath> path;
        Rectangle3D bounds;
        double resolution;

      public:
        Runner(PySimulation *self, PyObject *args, PyObject *kwds) :
          s(*self->s) {
          const char *kwlist[] = {"callback", "time", "threads", "reduce", 0};
          PyObject *cb = 0;

          if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OdIp", (char **)kwlist,
                                           &cb, &time, &threads, &reduce))
            THROW("Invalid arguments");

          setCallback(cb);

          if (!threads) threads = SystemInfo::instance().getCPUCount();
          if (!time) time = numeric_limits<double>::max();

          if (!s.path.isSet()) THROW("Missing tool path");

          path = s.path;
          bounds = s.project.getWorkpiece().getBounds();
          s.project.getWorkpiece().update(*path);
          resolution = s.project.getResolution();

          start();
        }


        // From Thread
        void run() {
          CAMotics::Simulation
            sim(path, bounds, resolution, time, RenderMode(), threads);

          auto surface = SimulationRun(sim).compute(*this);
          if (reduce) surface->reduce(*this);

          SmartPyGIL gil;
          s.surface = surface;
        }
      };


      set_task(self, new Runner(self, args, kwds));

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_surface(PySimulation *self) {
    try {
      PyJSONSink sink;
      self->s->surface->write(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_write_surface(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"filename", "binary", 0};
      const char *filename = 0;
      int binary = true;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p", (char **)kwlist,
                                       &filename, &binary))
        return 0;

      if (!filename) {
        PyErr_SetString(PyExc_RuntimeError, "Missing filename.");
        return 0;
      }

      s.surface->writeSTL(*SystemUtilities::oopen(filename), binary,
                          "CAMotics Surface", "");

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_is_running(PySimulation *self) {
    try {
      auto &task = self->s->task;

      if (task.isSet() && task->isRunning()) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_wait(PySimulation *self) {
    try {
      auto &task = self->s->task;

      if (task.isSet()) {
        task->join();
        task.release();
      }

      Py_RETURN_NONE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_interrupt(PySimulation *self) {
    try {
      auto &task = self->s->task;
      if (task.isSet()) task->stop();

      Py_RETURN_NONE;
    } CATCH_PYTHON;

    return 0;
  }


  PyMethodDef _methods[] = {
    {"open", (PyCFunction)_open, METH_VARARGS | METH_KEYWORDS,
     "Open a CAMotics project, GCode or TPL file."},
    {"set_resolution", (PyCFunction)_set_resolution,
     METH_VARARGS | METH_KEYWORDS, "Set simulation resolution and mode."},
    {"compute_path", (PyCFunction)_compute_path, METH_NOARGS,
     "Compute tool path"},
    {"get_path", (PyCFunction)_get_path, METH_NOARGS, ""},
    {"start", (PyCFunction)_start, METH_VARARGS | METH_KEYWORDS,
     "Start a simulation run."},
    {"get_surface", (PyCFunction)_get_surface, METH_NOARGS,
     "Get surface mesh."},
    {"write_surface", (PyCFunction)_write_surface, METH_VARARGS | METH_KEYWORDS,
     "Write surface STL to named file."},
    {"is_running", (PyCFunction)_is_running, METH_NOARGS,
     "Returns true if a task is running."},
    {"wait", (PyCFunction)_wait, METH_NOARGS, "Wait on running task."},
    {"interrupt", (PyCFunction)_interrupt, METH_NOARGS,
     "Interrupt running task."},
    {0}
  };
}


PyTypeObject SimulationType = {
  PyVarObject_HEAD_INIT(0, 0)
  "Simulation",              // tp_name
  sizeof(PySimulation),      // tp_basicsize
  0,                         // tp_itemsize
  (destructor)_dealloc,      // tp_dealloc
  0,                         // tp_print
  0,                         // tp_getattr
  0,                         // tp_setattr
  0,                         // tp_reserved
  0,                         // tp_repr
  0,                         // tp_as_number
  0,                         // tp_as_sequence
  0,                         // tp_as_mapping
  0,                         // tp_hash
  0,                         // tp_call
  0,                         // tp_str
  0,                         // tp_getattro
  0,                         // tp_setattro
  0,                         // tp_as_buffer
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,       // tp_flags
  "Simulation object",       // tp_doc
  0,                         // tp_traverse
  0,                         // tp_clear
  0,                         // tp_richcompare
  0,                         // tp_weaklistoffset
  0,                         // tp_iter
  0,                         // tp_iternext
  _methods,                  // tp_methods
  0,                         // tp_members
  0,                         // tp_getset
  0,                         // tp_base
  0,                         // tp_dict
  0,                         // tp_descr_get
  0,                         // tp_descr_set
  0,                         // tp_dictoffset
  (initproc)_init,           // tp_init
  0,                         // tp_alloc
  _new,                      // tp_new
};
