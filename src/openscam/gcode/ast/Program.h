/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_PROGRAM_H
#define OPENSCAM_PROGRAM_H

#include <cbang/SmartPointer.h>

#include <vector>

#include "Block.h"
#include "Entity.h"

#include <openscam/gcode/Processor.h>


namespace OpenSCAM {
  class Program : public Entity, public std::vector<cb::SmartPointer<Block> >,
                  public Processor {
  public:
    void process(Processor &processor);

    // From Entity
    void print(std::ostream &stream) const;

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);
  };
}

#endif // OPENSCAM_PROGRAM_H

