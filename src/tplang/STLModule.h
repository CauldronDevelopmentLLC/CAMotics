/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#ifndef TPLANG_CONTOUR_MODULE_H
#define TPLANG_CONTOUR_MODULE_H

#include "TPLContext.h"

#include <cbang/js/Module.h>
#include <cbang/io/InputSource.h>
#include <camotics/stl/STLReader.h>
#include <cbang/os/SystemUtilities.h>
#include <iostream>
#include <exception>
#include <sstream>
#include <vector>
#include <cmath>


namespace tplang {
  class STLModule : public cb::js::Module {
  
  public:
    STLModule() {
      define(*this);
      reader = NULL;
    }
    void define(cb::js::ObjectTemplate &exports);
    // Javascript call backs
    cb::js::Value makeContourCB(const cb::js::Arguments &args);
    
    CAMotics::STLReader * reader;

  };
}

#endif // TPLANG_CONTOUR_MODULE_H
