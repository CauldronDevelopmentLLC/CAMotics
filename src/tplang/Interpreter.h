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

#ifndef TPLANG_INTERPRETER_H
#define TPLANG_INTERPRETER_H

#include "TPLContext.h"
#include "GCodeLibrary.h"
#include "MatrixLibrary.h"
#include "DXFLibrary.h"
#include "ClipperLibrary.h"

#include <cbang/io/Reader.h>
#include <cbang/io/OutputSink.h>

#include <cbang/js/StdLibrary.h>

#include <ostream>


namespace tplang {
  class Interpreter : public cb::Reader {
    TPLContext &ctx;
    cb::js::StdLibrary stdLib;
    GCodeLibrary gcodeLib;
    MatrixLibrary matrixLib;
    DXFLibrary dxfLib;
    ClipperLibrary clipperLib;

  public:
    Interpreter(TPLContext &ctx);

    // From cb::Reader
    void read(const cb::InputSource &source);
  };
}

#endif // TPLANG_INTERPRETER_H

