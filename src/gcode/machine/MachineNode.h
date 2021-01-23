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

#include "MachineInterface.h"


namespace GCode {
  class MachineNode {
  protected:
    cb::SmartPointer<MachineInterface> next;

  public:
    MachineNode(const cb::SmartPointer<MachineInterface> &next = 0) :
      next(next) {}
    virtual ~MachineNode() {}

    void setNextNode(const cb::SmartPointer<MachineInterface> &next)
      {this->next = next;}
    const cb::SmartPointer<MachineInterface> &getNextNode() const {return next;}


    template <typename T>
    T &find() {
      T *ptr = dynamic_cast<T *>(this);
      if (ptr) return *ptr;

      MachineNode *node = dynamic_cast<MachineNode *>(next.get());
      if (!node) THROW("Not found");

      return node->find<T>();
    }
  };
}
