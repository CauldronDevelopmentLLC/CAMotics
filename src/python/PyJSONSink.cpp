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

#include "PyJSONSink.h"

using namespace cb;
using namespace std;


void PyJSONSink::writeNull() {
  JSON::NullSink::writeNull();
  Py_INCREF(Py_None);
  add(Py_None);
}


void PyJSONSink::writeBoolean(bool value) {
  JSON::NullSink::writeBoolean(value);
  PyObject *o = value ? Py_True : Py_False;
  Py_INCREF(o);
  add(o);
}


void PyJSONSink::write(double value) {
  JSON::NullSink::write(value);
  add(PyFloat_FromDouble(value));
}


void PyJSONSink::write(int8_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromLong(value));
}


void PyJSONSink::write(uint8_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromUnsignedLong(value));
}


void PyJSONSink::write(int16_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromLong(value));
}


void PyJSONSink::write(uint16_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromUnsignedLong(value));
}


void PyJSONSink::write(int32_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromLong(value));
}


void PyJSONSink::write(uint32_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromUnsignedLong(value));
}


void PyJSONSink::write(int64_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromLongLong(value));
}


void PyJSONSink::write(uint64_t value) {
  JSON::NullSink::write((double)value);
  add(PyLong_FromUnsignedLongLong(value));
}


void PyJSONSink::write(const string &value) {
  JSON::NullSink::write(value);
  add(createString(value));
}


void PyJSONSink::beginList(bool simple) {
  push(PyList_New(0));
  JSON::NullSink::beginList(false);
}


void PyJSONSink::beginAppend() {JSON::NullSink::beginAppend();}


void PyJSONSink::endList() {
  JSON::NullSink::endList();
  pop();
}


void PyJSONSink::beginDict(bool simple) {
  push(PyDict_New());
  JSON::NullSink::beginDict(simple);
}


void PyJSONSink::beginInsert(const string &key) {
  JSON::NullSink::beginInsert(key);
  this->key = key;
}


void PyJSONSink::endDict() {
  JSON::NullSink::endDict();
  pop();
}


PyObject *PyJSONSink::createString(const string &s) {
  return PyUnicode_FromStringAndSize(CPP_TO_C_STR(s), s.length());
}


void PyJSONSink::push(PyObject *o) {
  add(o);
  stack.push_back(o);
}


void PyJSONSink::pop() {stack.pop_back();}


void PyJSONSink::add(PyObject *o) {
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
