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

#include "PyLogger.h"
#include "Throw.h"

#include <cbang/log/Logger.h>

using namespace std;


PyLogger::PyLogger(PyObject *cb) : cb(cb) {
  Py_INCREF(cb);
  if (!PyCallable_Check(cb)) THROW("logger object not callable");
}


PyLogger::PyLogger(const PyLogger &o) : cb(o.cb) {Py_INCREF(cb);}


PyLogger::~PyLogger() {Py_DECREF(cb);}


PyLogger &PyLogger::operator=(const PyLogger &o) {
  cb = o.cb;
  Py_INCREF(cb);
  return *this;
}


streamsize PyLogger::write(const char *s, streamsize n) {
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
