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

#pragma once

#include <Python.h>


class PyPtr {
  PyObject *ptr = 0;

public:
  PyPtr(PyObject *ptr = 0) : ptr(ptr) {if (ptr) Py_INCREF(ptr);}
  ~PyPtr() {if (ptr) Py_DECREF(ptr);}


  PyObject *&get() {return ptr;}
  const PyObject *get() const {return ptr;}
  operator bool() const {return ptr;}


  PyPtr &operator=(const PyPtr &o) {
    if (ptr) Py_DECREF(ptr);
    ptr = o.ptr;
    if (ptr) Py_INCREF(ptr);
    return *this;
  }
};
