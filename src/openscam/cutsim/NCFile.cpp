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

#include "NCFile.h"

#include "Project.h"

#include <cbang/os/SystemUtilities.h>

using namespace OpenSCAM;
using namespace cb;
using namespace std;


NCFile::NCFile(Project &project, const string &filename) :
  project(project),
  absPath(SystemUtilities::absolute(project.getDirectory(), filename)),
  modTime(getTime()) {}


bool NCFile::isTPL() const {
  return String::endsWith(absPath, ".tpl");
}


void NCFile::setFilename(const string &filename) {
  absPath = SystemUtilities::absolute(filename);
}


string NCFile::getRelativePath() const {
  return SystemUtilities::relative(project.getDirectory(), absPath, 4);
}


bool NCFile::exists() const {
  return SystemUtilities::exists(absPath);
}


uint64_t NCFile::getTime() const {
  return SystemUtilities::exists(absPath) ?
    SystemUtilities::getModificationTime(absPath) : 0;
}
