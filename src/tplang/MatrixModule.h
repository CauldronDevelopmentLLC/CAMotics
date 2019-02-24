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


#include <gcode/machine/MachineEnum.h>
#include <gcode/machine/Transforms.h>

#include <cbang/js/NativeModule.h>


namespace tplang {
  class TPLContext;

  class MatrixModule :
    public cb::js::NativeModule, public GCode::MachineEnum {
    TPLContext &ctx;

  public:
    MatrixModule(TPLContext &ctx);

    // From cb::js::NativeModule
    void define(cb::js::Sink &exports);

    GCode::TransformStack &getTransformStack(const cb::js::Value &args);
    GCode::Transform &getTransform(const cb::js::Value &args);

    // Javascript call backs
    void pushMatrixCB(const cb::js::Value &args, cb::js::Sink &sink);
    void popMatrixCB(const cb::js::Value &args, cb::js::Sink &sink);
    void loadIdentityCB(const cb::js::Value &args, cb::js::Sink &sink);
    void scaleCB(const cb::js::Value &args, cb::js::Sink &sink);
    void translateCB(const cb::js::Value &args, cb::js::Sink &sink);
    void rotateCB(const cb::js::Value &args, cb::js::Sink &sink);
    void setMatrixCB(const cb::js::Value &args, cb::js::Sink &sink);
    void getMatrixCB(const cb::js::Value &args, cb::js::Sink &sink);

    void getXYZ(const cb::js::Value &args, cb::js::Sink &sink);
    void getX(const cb::js::Value &args, cb::js::Sink &sink);
    void getY(const cb::js::Value &args, cb::js::Sink &sink);
    void getZ(const cb::js::Value &args, cb::js::Sink &sink);

  protected:
    axes_t parseAxes(const cb::js::Value &args);
  };
}
