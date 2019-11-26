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

#pragma once


#include "Task.h"

#include <cbang/StdTypes.h>

#include <iosfwd> // streamsize
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/operations.hpp>


namespace CAMotics {
  class TaskFilter : public boost::iostreams::multichar_dual_use_filter {
    Task &task;
    uint64_t total;
    uint64_t count;

  public:
    struct category : boost::iostreams::multichar_dual_use_filter::category,
      boost::iostreams::flushable_tag {};

    TaskFilter(Task &task, uint64_t total) :
      task(task), total(total), count(0) {}

    void setTotal(uint64_t total) {this->total = total;}

    template<typename Source>
    std::streamsize read(Source &src, char *s, std::streamsize n) {
      n = boost::iostreams::read(src, s, n);
      if (0 < n) {
        count += n;
        if (total) task.update((double)count / total);
      }
      return n;
    }

    template<typename Sink>
    std::streamsize write(Sink &dest, const char *s, std::streamsize n) {
      n = boost::iostreams::write(dest, s, n);
      if (0 < n) {
        count += n;
        if (total) task.update((double)count / total);
      }
      return n;
    }

    template<typename Sink> bool flush(Sink &snk) {return true;}
  };
}
