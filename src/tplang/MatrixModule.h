/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#ifndef TPLANG_MATRIX_MODULE_H
#define TPLANG_MATRIX_MODULE_H

#include "TPLContext.h"
#include "MachineMatrix.h"
#include "MachineEnum.h"

#include <cbang/js/Module.h>


namespace tplang {
  class MatrixModule : public cb::js::Module, public MachineEnum {
    TPLContext &ctx;
    MachineMatrix &matrix;

  public:
    MatrixModule(TPLContext &ctx);

    void define(cb::js::ObjectTemplate &exports);

    // Javascript call backs
    cb::js::Value pushMatrixCB(const cb::js::Arguments &args);
    cb::js::Value popMatrixCB(const cb::js::Arguments &args);
    cb::js::Value loadIdentityCB(const cb::js::Arguments &args);
    cb::js::Value scaleCB(const cb::js::Arguments &args);
    cb::js::Value translateCB(const cb::js::Arguments &args);
    cb::js::Value rotateCB(const cb::js::Arguments &args);
    cb::js::Value setMatrixCB(const cb::js::Arguments &args);
    cb::js::Value getMatrixCB(const cb::js::Arguments &args);

    cb::js::Value getXYZ(const cb::js::Arguments &args);
    cb::js::Value getX(const cb::js::Arguments &args);
    cb::js::Value getY(const cb::js::Arguments &args);
    cb::js::Value getZ(const cb::js::Arguments &args);

  protected:
    axes_t parseMatrix(const cb::js::Arguments &args);
  };
}

#endif // TPLANG_MATRIX_MODULE_H

