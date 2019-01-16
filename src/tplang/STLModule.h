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


#include <cbang/js/NativeModule.h>


namespace tplang {
  class TPLContext;

  class STLModule : public cb::js::NativeModule {
    TPLContext &ctx;

  public:
    STLModule(TPLContext &ctx);

    // From cb::js::NativeModule
    void define(cb::js::Sink &exports);

    // Javascript callbacks
    void open(const cb::js::Value &args, cb::js::Sink &sink);
    void bounds(const cb::js::Value &args, cb::js::Sink &sink);
    void contour(const cb::js::Value &args, cb::js::Sink &sink);
  };
}
