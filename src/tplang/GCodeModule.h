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

#ifndef TPLANG_GCODE_MODULE_H
#define TPLANG_GCODE_MODULE_H

#include "TPLContext.h"
#include "MachineUnitAdapter.h"
#include "MachineEnum.h"

#include <cbang/js/Module.h>


namespace tplang {
  class GCodeModule : public cb::js::Module, public MachineEnum {
    TPLContext &ctx;
    MachineUnitAdapter &unitAdapter;

  public:
    GCodeModule(TPLContext &ctx) :
    ctx(ctx), unitAdapter(ctx.find<MachineUnitAdapter>()) {define(*this);}

    void define(cb::js::ObjectTemplate &exports);

    // Javascript call backs
    cb::js::Value gcodeCB(const cb::js::Arguments &args);
    cb::js::Value rapidCB(const cb::js::Arguments &args);
    cb::js::Value cutCB(const cb::js::Arguments &args);
    cb::js::Value arcCB(const cb::js::Arguments &args);
    cb::js::Value probeCB(const cb::js::Arguments &args);
    cb::js::Value dwellCB(const cb::js::Arguments &args);
    cb::js::Value feedCB(const cb::js::Arguments &args);
    cb::js::Value speedCB(const cb::js::Arguments &args);
    cb::js::Value toolCB(const cb::js::Arguments &args);
    cb::js::Value unitsCB(const cb::js::Arguments &args);
    cb::js::Value pauseCB(const cb::js::Arguments &args);
    cb::js::Value toolSetCB(const cb::js::Arguments &args);
    cb::js::Value positionCB(const cb::js::Arguments &args);

  protected:
    void parseAxes(const cb::js::Arguments &args, Axes &axes,
                   bool incremental = false);
  };
}

#endif // TPLANG_GCODE_MODULE_H

