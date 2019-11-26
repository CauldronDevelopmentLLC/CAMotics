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

#include <camotics/qt/QtApp.h>

#include <cbang/ApplicationMain.h>
#include <cbang/config.h>

// This causes Windows to not automatically create a console
#if defined(_WIN32) && !defined(__MINGW32__) && !defined(DEBUG)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

#ifdef HAVE_V8
#include <cbang/js/v8/JSImpl.h>
#endif

using namespace std;


int main(int argc, char *argv[]) {
#ifdef HAVE_V8
  cb::gv8::JSImpl::init(0, 0);
#endif
  return cb::doApplication<CAMotics::QtApp>(argc, argv);
}
