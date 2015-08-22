/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#include "Project.h"

#include "CutWorkpiece.h"
#include "Project.h"

#include <camotics/Geom.h>
#include <camotics/sim/ToolTable.h>
#include <camotics/cutsim/Sweep.h>

#include <cbang/os/SystemUtilities.h>
#include <cbang/time/Time.h>
#include <cbang/log/Logger.h>
#include <cbang/xml/XMLWriter.h>
#include <cbang/xml/XMLReader.h>
#include <cbang/config/EnumConstraint.h>
#include <cbang/debug/Debugger.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


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
  markDirty();
}


string Project::getDirectory() const {
  return filename.empty() ? SystemUtilities::getcwd() :
    SystemUtilities::dirname(filename);
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

  if (SystemUtilities::exists(_filename)) {
    XMLReader reader;
    reader.addFactory("tool_table", tools.get());
    reader.read(filename, &options);

    // Default workpiece
    if (!options["automatic-workpiece"].hasValue())
      options["automatic-workpiece"].
        setDefault(workpieceMin.empty() && workpieceMax.empty());

    // Load NC files
    files.clear();
    Option::strings_t ncFiles = options["nc-files"].toStrings();
    for (unsigned i = 0; i < ncFiles.size(); i++) {
      string relPath = decodeFilename(ncFiles[i]);
      addFile(SystemUtilities::absolute(getDirectory(), relPath));
    }
  }

  markClean();
}


void Project::save(const string &_filename) {
  setFilename(_filename);

  // Set nc-files option
  options["nc-files"].reset();
  for (files_t::iterator it = files.begin(); it != files.end(); it++)
    options["nc-files"].append((*it)->getRelativePath());

  SmartPointer<iostream> stream = SystemUtilities::open(filename, ios::out);
  XMLWriter writer(*stream, true);

  writer.startElement("camotics");
  writer.comment("Note, all values are in mm regardless of 'units' option.");
  options.write(writer, 0);
  tools->write(writer);
  writer.endElement("camotics");

  markClean();
}


const SmartPointer<NCFile> &Project::getFile(unsigned index) const {
  unsigned count = 0;

  for (iterator it = begin(); it != end(); it++)
    if (count++ == index) return *it;

  THROWS("Invalid file index " << index);
}


SmartPointer<NCFile> Project::findFile(const string &filename) const {
  string abs = SystemUtilities::absolute(filename);
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getAbsolutePath() == abs) return *it;
  return 0;
}


void Project::addFile(const string &filename) {
  string abs = SystemUtilities::absolute(filename);
  if (!findFile(abs).isNull()) return; // Duplicate

  files.push_back(new NCFile(*this, abs));
  markDirty();
}


void Project::removeFile(unsigned index) {
  unsigned count = 0;
  for (files_t::iterator it = files.begin(); it != files.end(); it++)
    if (count++ == index) {
      files.erase(it);
      markDirty();
      break;
    }
}


bool Project::checkFiles() {
  bool changed = false;

  if (watch && lastWatch < Time::now()) {
    for (iterator it = begin(); it != end(); it++)
      if ((*it)->changed()) {
        LOG_INFO(1, "File changed: " << (*it)->getRelativePath());
        changed = true;
      }

    lastWatch = Time::now();
  }

  return changed;
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

  // At least 2mm thick
  if (wpBounds.getHeight() < 2)
    wpBounds.add(Vector3R(bMin.x(), bMin.y(), bMin.z() - 2));

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
