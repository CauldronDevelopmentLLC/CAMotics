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

#include <gcode/Producer.h>

#include <cbang/SmartPointer.h>
#include <cbang/io/InputSource.h>

#include <vector>


namespace GCode {
  class Program;

  class ProducerStack : public Producer {
    std::vector<cb::SmartPointer<Producer> > producers;

  public:
    ~ProducerStack() {unwind();}

    bool empty() const {return producers.empty();}

    void push(const cb::SmartPointer<Producer> &producer);
    void push(const cb::InputSource &source);
    void push(Program &program);
    void push(const std::string &gcode, const std::string &path);

    cb::SmartPointer<Producer> peek();
    cb::SmartPointer<Producer> pop();

    void unwind();

    // From Producer
    bool hasMore() const;
    cb::SmartPointer<Block> next();
  };
}
