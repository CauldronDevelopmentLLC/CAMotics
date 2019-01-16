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

#include "File.h"

#include <cbang/os/SystemUtilities.h>

using namespace CAMotics::Project;
using namespace cb;
using namespace std;


File::File(const string &path) : path(path), modTime(getTime()) {}


bool File::isTPL() const {
  return String::endsWith(String::toLower(path), ".tpl");
}


bool File::isDXF() const {
  return String::endsWith(String::toLower(path), ".dxf");
}


void File::setPath(const string &_path) {
  path = SystemUtilities::absolute(_path);
}


string File::getRelativePath(const string &dir) const {
  return SystemUtilities::relative(dir, path, 4);
}


string File::getBasename() const {return SystemUtilities::basename(path);}
bool File::exists() const {return SystemUtilities::exists(path);}


uint64_t File::getTime() const {
  return exists() ? SystemUtilities::getModificationTime(path) : 0;
}
