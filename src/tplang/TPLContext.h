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

#ifndef TPLANG_TPLCONTEXT_H
#define TPLANG_TPLCONTEXT_H

#include "MachineAdapter.h"

#include <openscam/sim/ToolTable.h>

#include <cbang/js/Environment.h>
#include <cbang/config/Options.h>

namespace tplang {
  class TPLContext : public cb::js::Environment {
    std::vector<cb::SmartPointer<cb::js::Module> > modules;

  public:
    MachineInterface &machine;
    cb::SmartPointer<OpenSCAM::ToolTable> tools;

    TPLContext(std::ostream &out, MachineInterface &machine,
               const cb::SmartPointer<OpenSCAM::ToolTable> &tools);

    using cb::js::Environment::addModule;
    cb::js::Module &addModule(const cb::SmartPointer<cb::js::Module> &module);

    template <typename T>
    T &find() {
      MachineAdapter *adapter = dynamic_cast<MachineAdapter *>(&machine);
      if (!adapter) THROW("Not found");
      return adapter->find<T>();
    }
  };
}

#endif // TPLANG_TPLCONTEXT_H

