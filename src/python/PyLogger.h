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

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>


class PyLogger {
  PyObject *cb;

public:
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;

  PyLogger(PyObject *cb);
  PyLogger(const PyLogger &o);
  ~PyLogger();

  PyLogger &operator=(const PyLogger &o);
  std::streamsize write(const char *s, std::streamsize n);
};


class PyLoggerStream : public boost::iostreams::stream<PyLogger> {
public:
  PyLoggerStream(PyObject *cb) {this->open(PyLogger(cb));}
};
