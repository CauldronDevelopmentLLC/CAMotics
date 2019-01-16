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


#include <gcode/machine/MachineUnitAdapter.h>
#include <gcode/machine/MachineEnum.h>

#include <cbang/js/NativeModule.h>


namespace tplang {
  class TPLContext;

  class GCodeModule :
    public cb::js::NativeModule, public GCode::MachineEnum {
    TPLContext &ctx;
    GCode::MachineUnitAdapter *unitAdapter;

  public:
    GCodeModule(TPLContext &ctx);

    // From cb::js::NativeModule
    void define(cb::js::Sink &exports);

    GCode::MachineUnitAdapter &getUnitAdapter();

    // Javascript call backs
    void gcodeCB(const cb::js::Value &args, cb::js::Sink &sink);
    void rapidCB(const cb::js::Value &args, cb::js::Sink &sink);
    void cutCB(const cb::js::Value &args, cb::js::Sink &sink);
    void arcCB(const cb::js::Value &args, cb::js::Sink &sink);
    void probeCB(const cb::js::Value &args, cb::js::Sink &sink);
    void dwellCB(const cb::js::Value &args, cb::js::Sink &sink);
    void feedCB(const cb::js::Value &args, cb::js::Sink &sink);
    void speedCB(const cb::js::Value &args, cb::js::Sink &sink);
    void toolCB(const cb::js::Value &args, cb::js::Sink &sink);
    void unitsCB(const cb::js::Value &args, cb::js::Sink &sink);
    void pauseCB(const cb::js::Value &args, cb::js::Sink &sink);
    void positionCB(const cb::js::Value &args, cb::js::Sink &sink);
    void commentCB(const cb::js::Value &args, cb::js::Sink &sink);
    void messageCB(const cb::js::Value &args, cb::js::Sink &sink);
    void workpieceCB(const cb::js::Value &args, cb::js::Sink &sink);

  protected:
    int parseAxes(const cb::js::Value &args, GCode::Axes &position,
                  bool incremental = false);
  };
}
