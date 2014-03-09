/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_BLOCK_H
#define OPENSCAM_BLOCK_H

#include <cbang/SmartPointer.h>

#include <vector>

#include "Entity.h"

namespace OpenSCAM {
  class Word;
  class OCode;

  class Block : public Entity, public std::vector<cb::SmartPointer<Entity> > {
    typedef const std::vector<cb::SmartPointer<Entity> > Super_t;

    bool deleted;
    int line;

  public:
    Block(bool deleted, int line, const Super_t &children) :
      Super_t(children), deleted(deleted), line(line) {}

    bool isDeleted() const {return deleted;}
    int getLine() const {return line;}

    bool isEmpty() const {return !deleted && line == -1 && empty();}

    Word *findWord(char type, double number) const;
    Word *findWord(char type) const;
    OCode *findOCode() const;

    // From Entity
    void print(std::ostream &stream) const;
  };
}

#endif // OPENSCAM_BLOCK_H

