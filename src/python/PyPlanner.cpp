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

#include "PyPlanner.h"
#include "PyNameResolver.h"
#include "PyJSON.h"
#include "PyJSONSink.h"
#include "Catch.h"

#include <gcode/plan/Planner.h>

#include <cbang/String.h>
#include <cbang/io/StringStreamInputSource.h>

#include <limits>


namespace {
  typedef struct {
    PyObject_HEAD;
    GCode::Planner *planner;
  } PyPlanner;


  void _dealloc(PyPlanner *self) {
    if (self->planner) delete self->planner;
    Py_TYPE(self)->tp_free((PyObject *)self);
  }


  PyObject *_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return type->tp_alloc(type, 0);
  }


  int _init(PyPlanner *self, PyObject *args, PyObject *kwds) {
    try {
      self->planner = new GCode::Planner;
      return 0;
    } CATCH_PYTHON;

    return -1;
  }


  PyObject *_is_running(PyPlanner *self) {
    try {
      if (self->planner->isRunning()) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_is_synchronizing(PyPlanner *self) {
    try {
      if (self->planner->isSynchronizing()) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_resolver(PyPlanner *self, PyObject *args) {
    try {
      PyObject *cb;

      if (!PyArg_ParseTuple(args, "O", &cb)) return 0;

      if (cb == Py_None) self->planner->setResolver(0);
      else self->planner->setResolver(new PyNameResolver(cb));

      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_position(PyPlanner *self, PyObject *args) {
    try {
      GCode::Axes position(std::numeric_limits<double>::quiet_NaN());

      PyObject *_position = 0;

      if (!PyArg_ParseTuple(args, "O", &_position)) return 0;

      // Convert Python object to JSON and read Axes
      position.read(*PyJSON(_position).toJSON());

      self->planner->setPosition(position);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_load_string(PyPlanner *self, PyObject *args, PyObject *kwds) {
    try {
      GCode::PlannerConfig config;

      const char *kwlist[] = {"code", "config", "tpl", 0};
      const char *code;
      PyObject *_config = 0;
      int tpl = false;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|Op", (char **)kwlist,
                                       &code, &_config, &tpl)) return 0;

      // Convert Python object to JSON config
      if (_config) config.read(*PyJSON(_config).toJSON());

      cb::StringStreamInputSource stream(code, "<MDI>");
      self->planner->load(stream, config, tpl);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_load(PyPlanner *self, PyObject *args, PyObject *kwds) {
    try {
      GCode::PlannerConfig config;

      const char *kwlist[] = {"path", "config", 0};
      const char *path;
      PyObject *_config = 0;

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|O", (char **)kwlist,
                                       &path, &_config)) return 0;

      // Convert Python object to JSON config
      if (_config) config.read(*PyJSON(_config).toJSON());

      bool tpl = cb::String::endsWith(path, ".tpl");

      self->planner->load(std::string(path), config, tpl);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_has_more(PyPlanner *self) {
    try {
      if (self->planner->hasMore()) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_next(PyPlanner *self) {
    try {
      PyJSONSink sink;
      self->planner->next(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_set_active(PyPlanner *self, PyObject *args) {
    try {
      uint64_t id;

      if (!PyArg_ParseTuple(args, "K", &id)) return 0;

      self->planner->setActive(id);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_stop(PyPlanner *self) {
    try {
      self->planner->stop();
      Py_RETURN_NONE;
    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_restart(PyPlanner *self, PyObject *args, PyObject *kwds) {
    try {
      uint64_t id;
      GCode::Axes position(std::numeric_limits<double>::quiet_NaN());
      PyObject *_position = 0;
      const char *kwlist[] = {"id", "position", 0};

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "KO", (char **)kwlist, &id,
                                       &_position))
        return 0;

      // Convert Python object to JSON and read Axes
      position.read(*PyJSON(_position).toJSON());

      self->planner->restart(id, position);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_synchronize(PyPlanner *self, PyObject *args, PyObject *kwds) {
    try {
      double result = 0;
      const char *kwlist[] = {"result", 0};

      if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", (char **)kwlist, &result))
        return 0;

      self->planner->synchronize(result);
      Py_RETURN_NONE;

    } CATCH_PYTHON;

    return 0;
  }


  PyObject *_get_queue(PyPlanner *self) {
    try {
      PyJSONSink sink;
      self->planner->dumpQueue(sink);
      return sink.getRoot();
    } CATCH_PYTHON;

    return 0;
  }


  PyMethodDef _methods[] = {
    {"is_running", (PyCFunction)_is_running, METH_NOARGS,
     "True if the planner is active"},
    {"is_synchronizing", (PyCFunction)_is_synchronizing, METH_NOARGS,
     "True if the planner is synchronizing"},
    {"set_resolver", (PyCFunction)_set_resolver, METH_VARARGS,
     "Set name resolver callback"},
    {"set_position", (PyCFunction)_set_position, METH_VARARGS,
     "Set planner position"},
    {"load_string", (PyCFunction)_load_string, METH_VARARGS | METH_KEYWORDS,
     "Load GCode or TPL string"},
    {"load", (PyCFunction)_load, METH_VARARGS | METH_KEYWORDS,
     "Load GCode or TPL by filename"},
    {"has_more", (PyCFunction)_has_more, METH_NOARGS,
     "True if the planner has more data"},
    {"next", (PyCFunction)_next, METH_NOARGS, "Get next planner data"},
    {"set_active", (PyCFunction)_set_active, METH_VARARGS, "Tell the planner "
     "which plan ID is currently active"},
    {"stop", (PyCFunction)_stop, METH_NOARGS, "Stop planner"},
    {"restart", (PyCFunction)_restart, METH_VARARGS | METH_KEYWORDS,
     "Restart planner from given ID"},
    {"synchronize", (PyCFunction)_synchronize, METH_VARARGS | METH_KEYWORDS,
     "Continue with result synchronized command"},
    {"get_queue", (PyCFunction)_get_queue, METH_NOARGS, "Dump planner queue"},
    {0}
  };
}


PyTypeObject PlannerType = {
  PyVarObject_HEAD_INIT(0, 0)
  "Planner",                 // tp_name
  sizeof(PyPlanner),         // tp_basicsize
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
  "Planner object",          // tp_doc
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
