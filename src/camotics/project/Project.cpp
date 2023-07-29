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

#include "Project.h"
#include "XMLHandler.h"

#include <cbang/xml/XMLReader.h>
#include <cbang/os/SystemUtilities.h>

using namespace std;
using namespace cb;
using namespace CAMotics::Project;


Project::Project(const string &filename) :
  dirty(false), filename(filename), onDisk(false), files(getDirectory()),
  units(GCode::Units::METRIC),
  resolutionMode(ResolutionMode::RESOLUTION_MEDIUM), resolution(1) {
  if (!filename.empty()) load(filename);
}


Project::~Project() {}


void Project::setFilename(const string &_filename) {
  if (_filename.empty() || filename == _filename) return;
  filename = _filename;
  files.setDirectory(getDirectory());
  markDirty();
}


string Project::getDirectory() const {
  return filename.empty() ? SystemUtilities::getcwd() :
    SystemUtilities::dirname(filename);
}


string Project::getUploadFilename() const {
  string filename = SystemUtilities::basename(getFilename());
  return SystemUtilities::swapExtension(filename, "gc");
}


const SmartPointer<File> &Project::getFile(unsigned i) const {
  return files.get(i);
}


string Project::getFileRelativePath(unsigned i) const {
  return files.getRelativePath(i);
}


SmartPointer<File> Project::findFile(const string &path) const {
  return files.find(path);
}


void Project::addFile(const string &path) {
  if (filename.empty()) {
    filename = SystemUtilities::swapExtension(path, "camotics");
    files.setDirectory(SystemUtilities::dirname(path));
  }

  files.add(path);
  markDirty();
}


void Project::removeFile(unsigned i) {
  files.remove(i);
  markDirty();
}


void Project::setUnits(GCode::Units units) {
  if (units == getUnits()) return;
  this->units = units;
  markDirty();
}


void Project::setResolutionMode(CAMotics::ResolutionMode x) {
  if (x == resolutionMode) return;
  resolutionMode = x;
  markDirty();
}


double Project::getResolution() const {
  if (resolutionMode == ResolutionMode::RESOLUTION_MANUAL) return resolution;
  return computeResolution(resolutionMode, workpiece.getBounds());
}


void Project::setResolution(double resolution) {
  if (this->resolution == resolution) return;
  this->resolution = resolution;
  markDirty();
}


double Project::computeResolution(CAMotics::ResolutionMode mode,
                                  Rectangle3D bounds) {
  if (mode == ResolutionMode::RESOLUTION_MANUAL || bounds == Rectangle3D())
    return 1;

  double divisor;
  switch (mode) {
  case ResolutionMode::RESOLUTION_LOW: divisor = 100000; break;
  case ResolutionMode::RESOLUTION_HIGH: divisor = 5000000; break;
  case ResolutionMode::RESOLUTION_VERY_HIGH: divisor = 10000000; break;
  default: divisor = 250000; break; // Medium
  }

  return pow(bounds.getVolume() / divisor, 1.0 / 3.0);
}


void Project::load(const string &filename) {
  setFilename(filename);

  if (SystemUtilities::exists(filename)) {
    SmartPointer<JSON::Value> data;

    if (String::endsWith(String::toLower(filename), ".xml")) {
      JSON::Builder builder;
      CAMotics::Project::XMLHandler handler(builder);

      XMLReader().read(filename, &handler);
      data = builder.getRoot();

    } else data = JSON::Reader::parse(filename);

    read(*data);
    onDisk = true;
  }

  markClean();
}


void Project::save(const string &_filename) {
  setFilename(_filename);
  SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::out);
  JSON::Writer writer(*stream, 2, false);
  write(writer);
  onDisk = true;
  markClean();
}


void Project::read(const JSON::Value &value) {
  units = GCode::Units::parse(value.getString("units", "metric"));

  resolutionMode =
    ResolutionMode::parse(value.getString("resolution-mode", "medium"));
  resolution = value.getNumber("resolution", 1);

  if (value.has("tools")) tools.read(*value.get("tools"));
  if (value.has("workpiece")) workpiece.read(*value.get("workpiece"));
  if (value.has("files")) files.read(*value.get("files"));
}


void Project::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("units", String::toLower(units.toString()));
  sink.insert("resolution-mode", String::toLower(resolutionMode.toString()));
  sink.insert("resolution", resolution);

  sink.beginInsert("tools");
  tools.write(sink);

  sink.beginInsert("workpiece");
  workpiece.write(sink);

  sink.beginInsert("files");
  files.write(sink);

  sink.endDict();
}
