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

#include <cbang/json/NullSink.h>

#include <string>

#include <Python.h>


class PyJSONSink : public cb::JSON::NullSink {
  PyObject *root;

  typedef std::vector<PyObject *> stack_t;
  stack_t stack;
  std::string key;

public:
  PyJSONSink() : root(0) {}

  PyObject *getRoot() {return root;}

  // From cb::JSON::Sink
  void writeNull() override;
  void writeBoolean(bool value) override;
  void write(double   value) override;
  void write(int8_t   value) override;
  void write(uint8_t  value) override;
  void write(int16_t  value) override;
  void write(uint16_t value) override;
  void write(int32_t  value) override;
  void write(uint32_t value) override;
  void write(int64_t  value) override;
  void write(uint64_t value) override;
  void write(const std::string &value) override;
  void beginList(bool simple = false) override;
  void beginAppend() override;
  void endList() override;
  void beginDict(bool simple = false) override;
  void beginInsert(const std::string &key) override;
  void endDict() override;

  PyObject *createString(const std::string &s);
  void push(PyObject *o);
  void pop();
  void add(PyObject *o);
};
