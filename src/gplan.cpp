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

#include <Python.h>

#include <gcode/plan/Planner.h>

#include <cbang/log/Logger.h>
#include <cbang/json/JSON.h>
#include <cbang/io/StringStreamInputSource.h>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>

#include <limits>


#define CATCH_PYTHON                                            \
  catch (const cb::Exception &e) {                              \
    PyErr_SetString(PyExc_RuntimeError, SSTR(e).c_str());       \
  } catch (const std::exception &e) {                           \
    PyErr_SetString(PyExc_RuntimeError, e.what());              \
  } catch (...) {                                               \
    PyErr_SetString(PyExc_RuntimeError, "Unknown exception");   \
  }


class PyJSONSink : public cb::JSON::NullSink {
  PyObject *root;

  typedef std::vector<PyObject *> stack_t;
  stack_t stack;
  std::string key;

public:
  PyJSONSink() : root(0) {}

  PyObject *getRoot() {return root;}

  // From cb::JSON::Sink
  void writeNull() {
    cb::JSON::NullSink::writeNull();
    Py_INCREF(Py_None);
    add(Py_None);
  }


  void writeBoolean(bool value) {
    cb::JSON::NullSink::writeBoolean(value);
    PyObject *o = value ? Py_True : Py_False;
    Py_INCREF(o);
    add(o);
  }


  void write(double value) {
    cb::JSON::NullSink::write(value);
    add(PyFloat_FromDouble(value));
  }


  void write(int8_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromLong(value));
  }


  void write(uint8_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromUnsignedLong(value));
  }


  void write(int16_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromLong(value));
  }


  void write(uint16_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromUnsignedLong(value));
  }


  void write(int32_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromLong(value));
  }


  void write(uint32_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromUnsignedLong(value));
  }


  void write(int64_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromLongLong(value));
  }


  void write(uint64_t value) {
    cb::JSON::NullSink::write((double)value);
    add(PyLong_FromUnsignedLongLong(value));
  }


  void write(const std::string &value) {
    cb::JSON::NullSink::write(value);
    add(createString(value));
  }


  void beginList(bool simple = false) {
    push(PyList_New(0));
    cb::JSON::NullSink::beginList(false);
  }


  void beginAppend() {cb::JSON::NullSink::beginAppend();}


  void endList() {
    cb::JSON::NullSink::endList();
    pop();
  }


  void beginDict(bool simple = false) {
    push(PyDict_New());
    cb::JSON::NullSink::beginDict(simple);
  }


  void beginInsert(const std::string &key) {
    cb::JSON::NullSink::beginInsert(key);
    this->key = key;
  }


  void endDict() {
    cb::JSON::NullSink::endDict();
    pop();
  }


  PyObject *createString(const std::string &s) {
    return PyUnicode_FromStringAndSize(CPP_TO_C_STR(s), s.length());
  }


  void push(PyObject *o) {
    add(o);
    stack.push_back(o);
  }


  void pop() {stack.pop_back();}


  void add(PyObject *o) {
    if (!o) THROW("Cannot add null");

    if (!root) root = o;

    else if (inList()) {
      int ret = PyList_Append(stack.back(), o);

      Py_DECREF(o);

      if (ret) THROW("Append failed");

    } else if (inDict()) {
      PyObject *key = createString(this->key);

      int ret = PyDict_SetItem(stack.back(), key, o);

      Py_DECREF(key);
      Py_DECREF(o);

      if (ret) THROW("Insert failed");
    }
  }
};


void PyThrowIfError(const std::string &msg) {
  if (!PyErr_Occurred()) return;

  PyObject *err = PyObject_Str(PyErr_Occurred());
  const char *_errStr = PyUnicode_AsUTF8(err);
  std::string errStr = _errStr ? _errStr : "";
  Py_DECREF(err);

  THROW(msg << errStr);
}



class PyNameResolver : public GCode::NameResolver {
  PyObject *cb;

public:
  PyNameResolver(PyObject *cb) :cb(cb) {
    Py_INCREF(cb);
    if (!PyCallable_Check(cb)) THROW("get() object not callable");
  }


  ~PyNameResolver() {Py_DECREF(cb);}


  // From NameResolver
  double get(const std::string &name, GCode::Units units) {
    // Args
    PyObject *args = PyTuple_New(2);
    if (!args) THROW("Failed to allocate tuple");

    PyTuple_SetItem(args, 0, PyUnicode_FromString(name.c_str()));
    PyTuple_SetItem(args, 1, PyUnicode_FromString(units.toString()));

    // Call
    PyObject *result = PyObject_Call(cb, args, 0);
    Py_DECREF(args);

    // Convert result
    if (!result) THROW("Name resolver callback failed");
    double value = PyFloat_AsDouble(result);
    Py_DECREF(result);

    PyThrowIfError("Name resolver callback failed: ");

    return value;
  }
};


class PyLogger {
  PyObject *cb;

public:
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;

  PyLogger(PyObject *cb) : cb(cb) {
    Py_INCREF(cb);
    if (!PyCallable_Check(cb)) THROW("logger object not callable");
  }

  PyLogger(const PyLogger &o) : cb(o.cb) {Py_INCREF(cb);}


  ~PyLogger() {Py_DECREF(cb);}


  PyLogger &operator=(const PyLogger &o) {
    cb = o.cb;
    Py_INCREF(cb);
    return *this;
  }


  std::streamsize write(const char *s, std::streamsize n) {
    // Args
    PyObject *args = PyTuple_New(1);
    if (!args) THROW("Failed to allocate tuple");

    PyTuple_SetItem(args, 0, PyUnicode_FromStringAndSize(s, n));

    // Call
    PyObject *result = PyObject_Call(cb, args, 0);
    Py_DECREF(args);

    if (!result) THROW("Logger callback failed");
    Py_DECREF(result);

    PyThrowIfError("Logger callback failed: ");

    return n;
  }
};


class PyLoggerStream : public boost::iostreams::stream<PyLogger> {
public:
  PyLoggerStream(PyObject *cb) {this->open(PyLogger(cb));}
};


std::string PyUnicode_ToStdString(PyObject *o) {
  Py_ssize_t size;
  const char *s = PyUnicode_AsUTF8AndSize(o, &size);
  if (!s) THROW("Conversion from Python object to string failed");
  return std::string(s, size);
}


cb::SmartPointer<cb::JSON::Value> pyToJSON(PyObject *o) {
  if (!o) return cb::JSON::Null::instancePtr();

  if (PyDict_Check(o)) {
    cb::SmartPointer<cb::JSON::Value> dict = new cb::JSON::Dict;
    PyObject *items = PyMapping_Items(o);
    if (!items) THROW("Expected items");

    Py_ssize_t size = PySequence_Size(items);
    for (Py_ssize_t i = 0; i < size; i++) {
      PyObject *item = PySequence_GetItem(items, i);
      PyObject *key = PyTuple_GetItem(item, 0);
      PyObject *value = PyTuple_GetItem(item, 1);

      dict->insert(PyUnicode_ToStdString(key), pyToJSON(value));
    }

    Py_DECREF(items);
    return dict;
  }

  if (PyList_Check(o) || PyTuple_Check(o)) {
    cb::SmartPointer<cb::JSON::Value> list = new cb::JSON::List;

    Py_ssize_t size = PySequence_Size(o);
    for (Py_ssize_t i = 0; i < size; i++)
      list->append(pyToJSON(PySequence_GetItem(o, i)));

    return list;
  }

  if (PyBool_Check(o)) return o == Py_True ? cb::JSON::True::instancePtr() :
                         cb::JSON::False::instancePtr();
  if (PyUnicode_Check(o)) return new cb::JSON::String(PyUnicode_ToStdString(o));
  if (PyLong_Check(o)) return new cb::JSON::Number(PyLong_AsDouble(o));
  if (PyFloat_Check(o)) return new cb::JSON::Number(PyFloat_AsDouble(o));
  if (Py_None == o) return cb::JSON::Null::instancePtr();

  // Try to convert to string
  PyObject *str = PyObject_Str(o);
  if (str) return new cb::JSON::String(PyUnicode_ToStdString(str));

  // Get ASCII representation
  PyObject *repr = PyObject_ASCII(o);
  if (repr) return new cb::JSON::String(PyUnicode_ToStdString(repr));

  return cb::JSON::Null::instancePtr();
}


typedef struct {
  PyObject_HEAD;
  GCode::Planner *planner;
} PyPlanner;


static void _dealloc(PyPlanner *self) {
  if (self->planner) delete self->planner;
  Py_TYPE(self)->tp_free((PyObject *)self);
}


static PyObject *_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyPlanner *self = (PyPlanner *)type->tp_alloc(type, 0);
  return (PyObject *)self;
}


static int _init(PyPlanner *self, PyObject *args, PyObject *kwds) {
  try {
    self->planner = new GCode::Planner;

    return 0;
  } CATCH_PYTHON;

  return -1;
}


static PyObject *_is_running(PyPlanner *self) {
  try {
    if (self->planner->isRunning()) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  } CATCH_PYTHON;

  return 0;
}


static PyObject *_is_synchronizing(PyPlanner *self) {
  try {
    if (self->planner->isSynchronizing()) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  } CATCH_PYTHON;

  return 0;
}


static PyObject *_set_resolver(PyPlanner *self, PyObject *args) {
  try {
    PyObject *cb;

    if (!PyArg_ParseTuple(args, "O", &cb)) return 0;

    if (cb == Py_None) self->planner->setResolver(0);
    else self->planner->setResolver(new PyNameResolver(cb));

    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_set_logger(PyPlanner *self, PyObject *args) {
  try {
    PyObject *cb;
    int level = -1;
    const char *domainLevels = 0;

    if (!PyArg_ParseTuple(args, "O|is", &cb, &level, &domainLevels)) return 0;

    if (cb == Py_None) cb::Logger::instance().setScreenStream(0);
    else cb::Logger::instance().setScreenStream(new PyLoggerStream(cb));

    if (0 <= level) cb::Logger::instance().setVerbosity(level);
    if (domainLevels) cb::Logger::instance().setLogDomainLevels(domainLevels);

    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_set_position(PyPlanner *self, PyObject *args) {
  try {
    GCode::Axes position(std::numeric_limits<double>::quiet_NaN());

    PyObject *_position = 0;

    if (!PyArg_ParseTuple(args, "O", &_position)) return 0;

    // Convert Python object to JSON and read Axes
    position.read(*pyToJSON(_position));

    self->planner->setPosition(position);
    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_load_string(PyPlanner *self, PyObject *args) {
  try {
    GCode::PlannerConfig config;

    PyObject *_config = 0;
    const char *gcode;

    if (!PyArg_ParseTuple(args, "s|O", &gcode, &_config)) return 0;

    // Convert Python object to JSON config
    if (_config) config.read(*pyToJSON(_config));

    self->planner->load(cb::StringStreamInputSource(gcode, "<MDI>"), config);
    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_load(PyPlanner *self, PyObject *args) {
  try {
    GCode::PlannerConfig config;

    PyObject *_config = 0;
    const char *filename;

    if (!PyArg_ParseTuple(args, "s|O", &filename, &_config)) return 0;

    // Convert Python object to JSON config
    if (_config) config.read(*pyToJSON(_config));

    self->planner->load(std::string(filename), config);
    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_has_more(PyPlanner *self) {
  try {
    if (self->planner->hasMore()) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  } CATCH_PYTHON;

  return 0;
}


static PyObject *_next(PyPlanner *self) {
  try {
    PyJSONSink sink;
    self->planner->next(sink);
    return sink.getRoot();
  } CATCH_PYTHON;

  return 0;
}


static PyObject *_set_active(PyPlanner *self, PyObject *args) {
  try {
    uint64_t id;

    if (!PyArg_ParseTuple(args, "K", &id)) return 0;

    self->planner->setActive(id);
    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_stop(PyPlanner *self) {
  try {
    self->planner->stop();
    Py_RETURN_NONE;
  } CATCH_PYTHON;

  return 0;
}


static PyObject *_restart(PyPlanner *self, PyObject *args, PyObject *kwds) {
  try {
    uint64_t id;
    GCode::Axes position(std::numeric_limits<double>::quiet_NaN());
    PyObject *_position = 0;
    const char *kwlist[] = {"id", "position", 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "KO", (char **)kwlist, &id,
                                     &_position))
      return 0;

    // Convert Python object to JSON and read Axes
    position.read(*pyToJSON(_position));

    self->planner->restart(id, position);
    Py_RETURN_NONE;

  } CATCH_PYTHON;

  return 0;
}


static PyObject *_synchronize(PyPlanner *self, PyObject *args, PyObject *kwds) {
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


static PyObject *_get_queue(PyPlanner *self) {
  try {
    PyJSONSink sink;
    self->planner->dumpQueue(sink);
    return sink.getRoot();
  } CATCH_PYTHON;

  return 0;
}


static PyMethodDef _methods[] = {
  {"is_running", (PyCFunction)_is_running, METH_NOARGS,
   "True if the planner is active"},
  {"is_synchronizing", (PyCFunction)_is_synchronizing, METH_NOARGS,
   "True if the planner is synchronizing"},
  {"set_resolver", (PyCFunction)_set_resolver, METH_VARARGS,
   "Set name resolver callback"},
  {"set_logger", (PyCFunction)_set_logger, METH_VARARGS,
   "Set logger callback"},
  {"set_position", (PyCFunction)_set_position, METH_VARARGS,
   "Set planner position"},
  {"load_string", (PyCFunction)_load_string, METH_VARARGS, "Load GCode string"},
  {"load", (PyCFunction)_load, METH_VARARGS, "Load GCode by filename"},
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


static PyTypeObject PlannerType = {
  PyVarObject_HEAD_INIT(0, 0)
  "gplan.Planner",           // tp_name
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


static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT, "gplan", 0, -1, 0, 0, 0, 0, 0
};


PyMODINIT_FUNC PyInit_gplan() {
  // Configure logger
  cb::Logger::instance().setLogTime(false);
  cb::Logger::instance().setLogShortLevel(true);
  cb::Logger::instance().setLogColor(false);
  cb::Logger::instance().setLogPrefix(true);

  if (PyType_Ready(&PlannerType) < 0) return 0;

  PyObject *m = PyModule_Create(&module);
  if (!m) return 0;

  Py_INCREF(&PlannerType);
  PyModule_AddObject(m, "Planner", (PyObject *)&PlannerType);

  return m;
}
