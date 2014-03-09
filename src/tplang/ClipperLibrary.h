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

#ifndef TPLANG_CLIPPER_LIBRARY_H
#define TPLANG_CLIPPER_LIBRARY_H

#include "TPLContext.h"

#include <cbang/js/Library.h>


namespace tplang {
  class ClipperLibrary : public cb::js::Library {
  public:
    ClipperLibrary(TPLContext &ctx) : cb::js::Library(ctx) {}

    void add(cb::js::ObjectTemplate &tmpl);

    // Javascript call backs
    cb::js::Value offsetCB(const cb::js::Arguments &args);
  };
}

#endif // TPLANG_CLIPPER_LIBRARY_H

