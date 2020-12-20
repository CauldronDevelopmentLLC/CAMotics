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

#include "PyNameResolver.h"
#include "Throw.h"

using namespace std;


PyNameResolver::PyNameResolver(PyObject *cb) : cb(cb) {
  Py_INCREF(cb);
  if (!PyCallable_Check(cb)) THROW("get() object not callable");
}


PyNameResolver::~PyNameResolver() {Py_DECREF(cb);}


double PyNameResolver::get(const string &name, GCode::Units units) {
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
