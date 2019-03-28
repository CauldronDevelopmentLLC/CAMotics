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

#include "Files.h"

#include <cbang/os/SystemUtilities.h>
#include <cbang/json/Value.h>


using namespace CAMotics::Project;
using namespace cb;
using namespace std;


const SmartPointer<File> &Files::get(unsigned i) const {
  if (files.size() <= i) THROW("Invalid file index " << i);
  return files[i];
}


string Files::getRelativePath(unsigned i) const {
  return get(i)->getRelativePath(directory);
}


SmartPointer<File> Files::find(const string &_path) const {
  string path = SystemUtilities::absolute(directory, _path);

  for (unsigned i = 0; i < size(); i++)
    if (path == files[i]->getPath()) return files[i];

  return 0;
}


void Files::add(const string &path) {
  files.push_back(new File(SystemUtilities::absolute(path)));
}


void Files::read(const JSON::Value &value) {
  for (unsigned i = 0; i < value.size(); i++)
    add(SystemUtilities::absolute(directory, value.getString(i)));
}


void Files::write(JSON::Sink &sink) const {
  sink.beginList();

  for (unsigned i = 0; i < files.size(); i++)
    sink.append(get(i)->getRelativePath(directory));

  sink.endList();
}
