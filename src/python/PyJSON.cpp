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

#include "PyJSON.h"

using namespace cb;
using namespace std;


namespace {
  string PyUnicode_ToStdString(PyObject *o) {
    Py_ssize_t size;
    const char *s = PyUnicode_AsUTF8AndSize(o, &size);
    if (!s) THROW("Conversion from Python object to string failed");
    return string(s, size);
  }
}


SmartPointer<JSON::Value> PyJSON::toJSON() const {
  if (!o) return JSON::Null::instancePtr();

  if (PyDict_Check(o)) {
    SmartPointer<JSON::Value> dict = new JSON::Dict;
    PyObject *items = PyMapping_Items(o);
    if (!items) THROW("Expected items");

    Py_ssize_t size = PySequence_Size(items);
    for (Py_ssize_t i = 0; i < size; i++) {
      PyObject *item = PySequence_GetItem(items, i);
      PyObject *key = PyTuple_GetItem(item, 0);
      PyObject *value = PyTuple_GetItem(item, 1);

      dict->insert(PyUnicode_ToStdString(key), PyJSON(value).toJSON());
    }

    Py_DECREF(items);
    return dict;
  }

  if (PyList_Check(o) || PyTuple_Check(o)) {
    SmartPointer<JSON::Value> list = new JSON::List;

    Py_ssize_t size = PySequence_Size(o);
    for (Py_ssize_t i = 0; i < size; i++)
      list->append(PyJSON(PySequence_GetItem(o, i)).toJSON());

    return list;
  }

  if (PyBool_Check(o)) return o == Py_True ? JSON::True::instancePtr() :
                         JSON::False::instancePtr();
  if (PyUnicode_Check(o)) return new JSON::String(PyUnicode_ToStdString(o));
  if (PyLong_Check(o)) return new JSON::Number(PyLong_AsDouble(o));
  if (PyFloat_Check(o)) return new JSON::Number(PyFloat_AsDouble(o));
  if (Py_None == o) return JSON::Null::instancePtr();

  // Try to convert to string
  PyObject *str = PyObject_Str(o);
  if (str) return new JSON::String(PyUnicode_ToStdString(str));

  // Get ASCII representation
  PyObject *repr = PyObject_ASCII(o);
  if (repr) return new JSON::String(PyUnicode_ToStdString(repr));

  return JSON::Null::instancePtr();
}
