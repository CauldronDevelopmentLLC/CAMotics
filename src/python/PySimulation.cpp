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

#include "PyPtr.h"
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

#include <strings.h>

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


  PyObject *_is_metric(PySimulation *self) {
    if (self->s->project.isMetric()) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }


  PyObject *_set_metric(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      const char *kwlist[] = {"metric", 0};
      int metric = true;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "|p", (char **)kwlist,
                                       &metric))
        return 0;

      auto units = metric ? GCode::Units::METRIC : GCode::Units::IMPERIAL;
      self->s->project.setUnits(units);

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_resolution(PySimulation *self) {
    try {
      auto &p = self->s->project;
      string mode = p.getResolutionMode().toString();

      return Py_BuildValue("ds", p.getResolution(), mode.data());
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_resolution(PySimulation *self, PyObject *args,
                            PyObject *kwds) {
    try {
      auto &p = self->s->project;

      const char *kwlist[] = {"resolution", 0};
      PyObject *res = 0;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &res))
        return 0;

      if (PyFloat_Check(res)) {
        p.setResolutionMode(ResolutionMode::RESOLUTION_MANUAL);
        p.setResolution(PyFloat_AsDouble(res));

      } else if (PyUnicode_Check(res)) {
        Py_ssize_t size;
        const char *mode = PyUnicode_AsUTF8AndSize(res, &size);
        if (!mode) THROW("Conversion from Python object to string failed");

        p.setResolutionMode(ResolutionMode::parse(string(mode, size)));

      } else THROW("Invalid argument type");

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_tools(PySimulation *self) {
    try {
      PyJSONSink sink;
      self->s->project.getTools().write(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_tools(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"tools", 0};
      PyObject *tools = 0;

      if (!PyArg_ParseTupleAndKeywords
          (args, kwds, "O", (char **)kwlist, &tools))
        return 0;

      if (!tools) THROW("``tools`` not set'");
      s.project.getTools().read(*PyJSON(tools).toJSON());

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_tool(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      auto &tools = self->s->project.getTools();

      const char *kwlist[] = {
        "number", "metric", "shape", "length", "diameter", "snub",
        "description", 0};
      unsigned number = 1;
      int metric = true;
      const char *shape = 0;
      double length = 0;
      double diameter = 0;
      double snub = 0;
      const char *desc = 0;

      if (!PyArg_ParseTupleAndKeywords
          (args, kwds, "Ipsdd|ds", (char **)kwlist, &number, &metric, &shape,
           &length, &diameter, &snub, &desc))
        return 0;

      auto units = metric ? GCode::Units::METRIC : GCode::Units::IMPERIAL;
      GCode::Tool t(number, 0, units);
      t.setShape(GCode::ToolShape::parse(shape));
      t.setLength(length);
      t.setDiameter(diameter);
      t.setSnubDiameter(snub);
      if (desc) t.setDescription(desc);

      tools.set(t);

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_workpiece(PySimulation *self) {
    try {
      PyJSONSink sink;
      self->s->project.getWorkpiece().write(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_workpiece(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"min", "max", "automatic", "margin", 0};
      double minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;
      int automatic = false;
      double margin = 5;

      if (!PyArg_ParseTupleAndKeywords
          (args, kwds, "|(ddd)(ddd)pd", (char **)kwlist, &minX, &minY, &minZ,
           &maxX, &maxY, &maxZ, &automatic, &margin))
        return 0;

      auto &w = s.project.getWorkpiece();

      if (automatic) {
        w.setAutomatic(true);
        w.setMargin(margin);

      } else {
        Rectangle3D bounds(Vector3D(minX, minY, minZ),
                           Vector3D(maxX, maxY, maxZ));
        w.setAutomatic(false);
        w.setBounds(bounds);
      }

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_compute_path(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      class Runner : public PyTask {
        Simulation &s;
        string gcode;
        string tpl;
        ToolPathTask task;

      public:
        Runner(PySimulation *self, const string &gcode, const string &tpl,
               GCode::PlannerConfig *config) :
          s(*self->s), gcode(gcode), tpl(tpl), task(s.project, config) {
          start();
        }

        // From Thread
        void run() {
          if (!gcode.empty()) task.runGCodeString(gcode);
          else if (!tpl.empty()) task.runTPLString(tpl);
          else task.run();

          SmartPyGIL gil;
          s.path = task.getPath();
        }
      };

      const char *kwlist[] = {"gcode", "tpl", "config", 0};
      const char *gcode = 0;
      const char *tpl = 0;
      PyObject *config = 0;

      if (!PyArg_ParseTupleAndKeywords
          (args, kwds, "|ssO", (char **)kwlist, &gcode, &tpl, &config))
        return 0;

      if (gcode && tpl) THROW("Cannot set both ``gcode`` and ``tpl``");

      GCode::PlannerConfig planConfig;
      if (config) planConfig.read(*PyJSON(config).toJSON());

      set_task(self, new Runner(self, gcode ? gcode : "", tpl ? tpl : "",
                                config ? &planConfig : 0));

      Py_RETURN_NONE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_path(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      const char *kwlist[] = {"path", 0};
      PyObject *path = 0;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &path))
        return 0;

      self->s->path->read(*PyJSON(path).toJSON());

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


  void call_done(PyPtr &done, bool success) {
    if (!done) return;

    try {
      PyObject *args = PyTuple_New(1);
      if (!args) THROW("Failed to allocate tuple");
      PyTuple_SetItem(args, 0, success ? Py_True : Py_False);

      PyObject *result = PyObject_Call(done.get(), args, 0);
      Py_DECREF(args);
      if (result) Py_DECREF(result);
    } CATCH_ERROR;
  }


  PyObject *_start(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      class Runner : public PyTask {
        Simulation &s;
        double time = 0;
        unsigned threads = 0;
        int reduce = false;
        PyPtr done;

        SmartPointer<GCode::ToolPath> path;
        Rectangle3D bounds;
        double resolution;

      public:
        Runner(PySimulation *self, PyObject *args, PyObject *kwds) :
          s(*self->s) {
          try {
            const char *kwlist[] =
              {"callback", "time", "threads", "reduce", "done", 0};
            PyObject *cb = 0;
            PyObject *done = 0;

            if (!PyArg_ParseTupleAndKeywords(
                  args, kwds, "|OdIpO", (char **)kwlist, &cb, &time, &threads,
                  &reduce, &done))
              THROW("Invalid arguments");

            this->done = done;
            setCallback(cb);

            if (!threads) threads = SystemInfo::instance().getCPUCount();
            if (!time) time = numeric_limits<double>::max();

            if (!s.path.isSet()) THROW("Missing tool path");

            path = s.path;
            s.project.getWorkpiece().update(*path);
            bounds = s.project.getWorkpiece().getBounds();
            resolution = s.project.getResolution();

            start();

          } catch (...) {
            call_done(done, false);
            throw;
          }
        }


        // From Thread
        void run() {
          try {
            CAMotics::Simulation
              sim(path, 0, 0, bounds, resolution, time, RenderMode(), threads);

            auto surface = SimulationRun(sim).compute(*this);
            if (reduce) surface->reduce(*this);

            SmartPyGIL gil;
            s.surface = surface;

            call_done(done, surface.isSet());
            return;
          } CATCH_ERROR;

          SmartPyGIL gil;
          call_done(done, false);
        }
      };

      set_task(self, new Runner(self, args, kwds));

      Py_RETURN_NONE;

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


  PyObject *_get_surface(PySimulation *self, PyObject *args, PyObject *kwds) {
    try {
      Simulation &s = *self->s;

      const char *kwlist[] = {"format", 0};
      const char *format = "binary";

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", (char **)kwlist,
                                       &format))
        return 0;


      if (strcasecmp(format, "python") == 0) {
        PyJSONSink sink;
        s.surface->write(sink);
        return sink.getRoot();
      }

      bool binary;
      if (strcasecmp(format, "binary") == 0) binary = true;
      else if (strcasecmp(format, "ascii") == 0) binary = false;
      else THROW("Invalid format");

      if (s.surface.isNull()) Py_RETURN_NONE;

      ostringstream str;
      s.surface->writeSTL(str, binary, "CAMotics Surface", "");
      string data = str.str();

      return PyBytes_FromStringAndSize(data.data(), data.length());

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
        task->wait();
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
    {"is_metric", (PyCFunction)_is_metric, METH_NOARGS,
     "Return true if project is in metric mode."},
    {"set_metric", (PyCFunction)_set_metric,
     METH_VARARGS | METH_KEYWORDS, "Enable or disable metric mode."},
    {"get_resolution", (PyCFunction)_get_resolution, METH_NOARGS,
     "Get resolution and mode."},
    {"set_resolution", (PyCFunction)_set_resolution,
     METH_VARARGS | METH_KEYWORDS, "Set simulation resolution."},
    {"get_tools", (PyCFunction)_get_tools, METH_NOARGS, "Get tool table."},
    {"set_tools", (PyCFunction)_set_tools,
     METH_VARARGS | METH_KEYWORDS, "Set tool table."},
    {"set_tool", (PyCFunction)_set_tool,
     METH_VARARGS | METH_KEYWORDS, "Set tool table."},
    {"get_workpiece", (PyCFunction)_get_workpiece, METH_NOARGS,
     "Get workpiece dimensions."},
    {"set_workpiece", (PyCFunction)_set_workpiece,
     METH_VARARGS | METH_KEYWORDS, "Set workpiece dimensions."},
    {"compute_path", (PyCFunction)_compute_path, METH_VARARGS | METH_KEYWORDS,
     "Compute tool path"},
    {"set_path", (PyCFunction)_set_path, METH_VARARGS | METH_KEYWORDS,
     "Set tool path."},
    {"get_path", (PyCFunction)_get_path, METH_NOARGS, ""},
    {"start", (PyCFunction)_start, METH_VARARGS | METH_KEYWORDS,
     "Start a simulation run."},
    {"write_surface", (PyCFunction)_write_surface, METH_VARARGS | METH_KEYWORDS,
     "Write surface STL to named file."},
    {"get_surface", (PyCFunction)_get_surface, METH_VARARGS | METH_KEYWORDS,
     "Get surface as STL data or Python object."},
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
