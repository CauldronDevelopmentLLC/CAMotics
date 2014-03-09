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

#ifndef OPENSCAM_PROJECT_H
#define OPENSCAM_PROJECT_H

#include "ResolutionMode.h"

#include <openscam/sim/ToolUnits.h>

#include <cbang/config/OptionProxy.h>
#include <cbang/util/OrderedDict.h>

#include <openscam/Geom.h>


namespace OpenSCAM {
  class ToolPath;
  class CutWorkpiece;
  class ToolTable;
  class ToolPath;

  class Project {
    cb::OptionProxy options;
    std::string filename;

    cb::SmartPointer<ToolTable> tools;

    double resolution;

    double workpieceMargin;
    std::string workpieceMin;
    std::string workpieceMax;

    typedef cb::OrderedDict<uint64_t> files_t;
    files_t files;
    bool watch;
    uint64_t lastWatch;

    bool dirty;

  public:
    Project(cb::Options &options, const cb::SmartPointer<ToolTable> &tools,
            const std::string &filename = std::string());
    ~Project();

    bool isDirty() const {return dirty;}
    void markDirty();
    void markClean() {dirty = false;}

    const std::string &getFilename() const {return filename;}
    void setFilename(const std::string &filename);

    std::string makeRelative(const std::string &path) const;
    std::string makeAbsolute(const std::string &path) const;

    const cb::SmartPointer<ToolTable> &getToolTable() const {return tools;}

    bool hasNotes() const {return options["notes"].hasValue();}
    const std::string &getNotes() const {return options["notes"];}
    void setNotes(const std::string &notes) {options["notes"].set(notes);}

    void setUnits(ToolUnits units);
    ToolUnits getUnits() const;

    ResolutionMode getResolutionMode() const;
    void setResolutionMode(ResolutionMode x);
    double getResolution() const {return resolution;}
    void setResolution(double x);
    void updateResolution();

    void load(const std::string &filename);
    void save(const std::string &filename = std::string());

    void addFile(const std::string &filename);
    void removeFile(unsigned i);
    std::vector<std::string> getRelativeFiles() const;
    std::vector<std::string> getAbsoluteFiles() const;
    void setFiles(std::vector<std::string> &files);

    void updateAutomaticWorkpiece(ToolPath &path);
    bool getAutomaticWorkpiece() const;
    void setAutomaticWorkpiece(bool x);

    double getWorkpieceMargin() const {return workpieceMargin;}
    void setWorkpieceMargin(double x);

    void setWorkpieceBounds(const Rectangle3R &bounds);
    Rectangle3R getWorkpieceBounds() const;

    bool checkFiles();

    static std::string encodeFilename(const std::string &filename);
    static std::string decodeFilename(const std::string &filename);
  };
}

#endif // OPENSCAM_PROJECT_H

