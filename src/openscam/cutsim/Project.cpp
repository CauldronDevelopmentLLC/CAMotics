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

#include "Project.h"

#include "CutWorkpiece.h"
#include "Project.h"

#include <openscam/Geom.h>
#include <openscam/sim/ToolTable.h>
#include <openscam/cutsim/Sweep.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/time/Time.h>
#include <cbang/log/Logger.h>
#include <cbang/xml/XMLWriter.h>
#include <cbang/xml/XMLReader.h>
#include <cbang/config/EnumConstraint.h>
#include <cbang/debug/Debugger.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Project::Project(Options &_options, const std::string &filename) :
  options(_options), filename(filename), tools(new ToolTable),
  resolution(1), workpieceMargin(5), watch(true), lastWatch(0), dirty(false) {

  options.setAllowReset(true);

  options.pushCategory("Project");
  options.add("units", "Units used in project measurement",
              new EnumConstraint<ToolUnits>)->setDefault("mm");
  options.popCategory();

  options.pushCategory("Renderer");
  options.add("resolution-mode", "Automatically compute a reasonable renderer "
              "grid resolution.  Valid values are 'low', 'medium', 'high', "
              "'manual'.  If 'manual' then 'resolution' will be used.",
              new EnumConstraint<ResolutionMode>)->setDefault("medium");
  options.addTarget("resolution", resolution, "Renderer grid resolution");
  options.popCategory();

  options.pushCategory("NC Files");
  options.addTarget("watch", watch, "Watch input files for changes and "
                    "automatically reload");
  options.add("nc-files", "TPL/GCode files")->setType(Option::STRINGS_TYPE);
  options.popCategory();

  options.pushCategory("Workpiece");
  options.add("automatic-workpiece", "Automatically compute a cuboid "
              "workpiece based on the tool path boundary");
  options.addTarget("workpiece-margin", workpieceMargin,
                    "Percent margin around automatic workpiece");
  options.addTarget("workpiece-min", workpieceMin,
                    "Minimum bound of cuboid workpiece");
  options.addTarget("workpiece-max", workpieceMax,
                    "Maximum bound of cuboid workpiece");
  options.popCategory();

  if (!filename.empty()) load(filename);
}


Project::~Project() {}


void Project::markDirty() {
  dirty = true;
}


void Project::setFilename(const string &_filename) {
  if (_filename.empty() || filename == _filename) return;
  filename = _filename;

  vector<string> absFiles = getAbsoluteFiles();

  Option &option = options["nc-files"];
  option.reset();

  for (unsigned i = 0; i < absFiles.size(); i++)
    option.append(encodeFilename(makeRelative(absFiles[i])));
}


string Project::makeRelative(const string &path) const {
  string dir = filename.empty() ?
    SystemUtilities::getcwd() : SystemUtilities::dirname(filename);
  return SystemUtilities::relative(dir, path, 4);
}


string Project::makeAbsolute(const string &path) const {
  string dir = filename.empty() ?
    SystemUtilities::getcwd() : SystemUtilities::dirname(filename);
  return SystemUtilities::makeRelative(dir, path);
}


void Project::setUnits(ToolUnits units) {
  if (units == getUnits()) return;
  options["units"].set(units.toString());
  markDirty();
}


ToolUnits Project::getUnits() const {
  return ToolUnits::parse(options["units"]);
}


ResolutionMode Project::getResolutionMode() const {
  return ResolutionMode::parse(options["resolution-mode"]);
}


void Project::setResolutionMode(ResolutionMode x) {
  if (x == getResolutionMode()) return;

  options["resolution-mode"].set(x.toString());
  markDirty();
  updateResolution();
}


void Project::setResolution(double x) {
  if (x == getResolution()) return;

  options["resolution"].set(x);

  if (getResolutionMode() == ResolutionMode::RESOLUTION_MANUAL) markDirty();
}


void Project::updateResolution() {
  if (getResolutionMode() == ResolutionMode::RESOLUTION_MANUAL) return;

  Rectangle3R wpBounds = getWorkpieceBounds();
  if (wpBounds == Rectangle3R()) return;

  double divisor;
  switch (getResolutionMode()) {
  case ResolutionMode::RESOLUTION_LOW: divisor = 100000; break;
  case ResolutionMode::RESOLUTION_HIGH: divisor = 5000000; break;
  case ResolutionMode::RESOLUTION_VERY_HIGH: divisor = 10000000; break;
  default: divisor = 250000; break; // Medium
  }

  setResolution(pow(wpBounds.getVolume() / divisor, 1.0 / 3.0));
}


void Project::load(const string &_filename) {
  setFilename(_filename);

  XMLReader reader;
  reader.addFactory("tool_table", tools.get());
  reader.read(filename, &options);

  // Default workpiece
  if (!options["automatic-workpiece"].hasValue())
    options["automatic-workpiece"].
      setDefault(workpieceMin.empty() && workpieceMax.empty());

  // Load NC files
  Option::strings_t ncFiles = options["nc-files"].toStrings();
  options["nc-files"].reset();
  for (unsigned i = 0; i < ncFiles.size(); i++)
    addFile(decodeFilename(ncFiles[i]));

  markClean();
}


void Project::save(const string &_filename) {
  setFilename(_filename);

  SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::out);
  XMLWriter writer(*stream, true);

  writer.startElement("openscam");
  writer.comment("Note, all values are in mm regardless of 'units' option.");
  options.write(writer, 0);
  tools->write(writer);
  writer.endElement("openscam");

  markClean();
}


void Project::addFile(const string &path) {
  string abs = makeAbsolute(path);
  if (files.has(abs)) return; // Duplicate
  files[abs] = SystemUtilities::getModificationTime(abs);
  options["nc-files"].append(encodeFilename(makeRelative(abs)));
  markDirty();
}


void Project::removeFile(unsigned i) {
  vector<string> files = getRelativeFiles();
  for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
    if (!i--) {
      files.erase(it);
      setFiles(files);
      break;
    }
  markDirty();
}


vector<string> Project::getRelativeFiles() const {
  vector<string> files = options["nc-files"].toStrings();
  vector<string> result;

  for (unsigned i = 0; i < files.size(); i++)
    result.push_back(decodeFilename(files[i]));

  return result;
}


vector<string> Project::getAbsoluteFiles() const {
  vector<string> absFiles;

  for (unsigned i = 0; i < files.size(); i++)
    absFiles.push_back(files.keyAt(i));

  return absFiles;
}


void Project::setFiles(vector<string> &files) {
  options["nc-files"].reset();
  this->files.clear();
  for (unsigned i = 0; i < files.size(); i++) addFile(files[i]);
}


void Project::updateAutomaticWorkpiece(ToolPath &path) {
  if (!getAutomaticWorkpiece()) return;
  setAutomaticWorkpiece(true);
  Rectangle3R wpBounds;

  // Guess workpiece bounds from cutting moves
  vector<SmartPointer<Sweep> > sweeps;
  vector<Rectangle3R> bboxes;

  for (unsigned i = 0; i < path.size(); i++) {
    const Move &move = path.at(i);

    if (move.getType() != MoveType::MOVE_RAPID) {
      unsigned tool = move.getTool();

      if (sweeps.size() <= tool) sweeps.resize(tool + 1);
      if (sweeps[tool].isNull()) sweeps[tool] = tools->get(tool)->getSweep();

      sweeps[tool]->getBBoxes(move.getStartPt(), move.getEndPt(), bboxes, 0);
    }
  }

  for (unsigned i = 0; i < bboxes.size(); i++) wpBounds.add(bboxes[i]);

  if (wpBounds == Rectangle3R()) return;

  // Start from z = 0
  Vector3R bMin = wpBounds.getMin();
  Vector3R bMax = wpBounds.getMax();
  wpBounds = Rectangle3R(bMin, Vector3R(bMax.x(), bMax.y(), 0));

  // At least a height of 2
  if (wpBounds.getHeight() < 2)
    wpBounds = Rectangle3R(Vector3R(bMin.x(), bMin.y(), bMax.z() - 2), bMax);

  if (wpBounds.isReal()) {
    // Margin
    Vector3R margin =
      wpBounds.getDimensions() * getWorkpieceMargin() / 100.0;
    wpBounds.add(wpBounds.getMin() - margin);
    wpBounds.add(wpBounds.getMax() + Vector3R(margin.x(), margin.y(), 0));

    setWorkpieceBounds(wpBounds);
  }
}


bool Project::getAutomaticWorkpiece() const {
  return (options["automatic-workpiece"].hasValue() &&
          options["automatic-workpiece"].toBoolean()) ||
    (workpieceMin.empty() && workpieceMax.empty());
}


void Project::setAutomaticWorkpiece(bool x) {
  if (getAutomaticWorkpiece() != x) markDirty();
  options["automatic-workpiece"].set(x);
}


void Project::setWorkpieceMargin(double x) {
  if (getWorkpieceMargin() == x) return;
  options["workpiece-margin"].set(x);
  markDirty();
}


void Project::setWorkpieceBounds(const Rectangle3R &bounds) {
  options["workpiece-min"].set(bounds.getMin().toString());
  options["workpiece-max"].set(bounds.getMax().toString());
  updateResolution();
  if (!getAutomaticWorkpiece()) markDirty();
}


Rectangle3R Project::getWorkpieceBounds() const {
  Vector3R wpMin = workpieceMin.empty() ? Vector3R() : Vector3R(workpieceMin);
  Vector3R wpMax = workpieceMax.empty() ? Vector3R() : Vector3R(workpieceMax);
  return Rectangle3R(wpMin, wpMax);
}


bool Project::checkFiles() {
  if (watch && lastWatch < Time::now()) {
    for (unsigned i = 0; i < files.size(); i++) {
      uint64_t fileTime = SystemUtilities::getModificationTime(files.keyAt(i));
      if (files[i] < fileTime) {
        files[i] = fileTime;
        LOG_INFO(1, "File changed: " << files.keyAt(i));
        return true;
      }
    }

    lastWatch = Time::now();
  }

  return false;
}


string Project::encodeFilename(const string &filename) {
  string result;

  for (unsigned i = 0; i < filename.size(); i++)
    switch (filename[i]) {
    case '\t': result += "%07"; break;
    case '\n': result += "%0A"; break;
    case '\v': result += "%0B"; break;
    case '\r': result += "%0D"; break;
    case '%': result += "%25"; break;
    case ' ': result += "%20"; break;
    default: result += filename[i]; break;
    }

  return result;
}


string Project::decodeFilename(const string &filename) {
  string result;

  for (unsigned i = 0; i < filename.size(); i++)
    if (filename[i] == '%' && i < filename.size() - 2) {
      result += (char)String::parseU8("0x" + filename.substr(i + 1, 2));
      i += 2;

    } else result += filename[i];

  return result;
}
