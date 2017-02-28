/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_PRINTER_H
#define CAMOTICS_PRINTER_H

#include "Processor.h"

#include <ostream>

namespace CAMotics {
  class Printer : public Processor {
  protected:
    std::ostream &stream;

    bool addComments;
    bool removeBlankLines;

  public:
    Printer(std::ostream &stream) :
      stream(stream), addComments(false), removeBlankLines(false) {}

    void setAddComments(bool x) {addComments = x;}
    void setRemoveBlankLines(bool x) {removeBlankLines = x;}

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);
  };
}

#endif // CAMOTICS_PRINTER_H
