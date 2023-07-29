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

#include "Workpiece.h"
#include "Files.h"
#include "ResolutionMode.h"

#include <camotics/render/RenderMode.h>

#include <gcode/Units.h>
#include <gcode/ToolTable.h>

#include <cbang/config/OptionProxy.h>
#include <cbang/json/JSON.h>



namespace GCode {
  class ToolTable;
  class ToolPath;
}


namespace CAMotics {
  namespace Project {
    class Project : public cb::JSON::Serializable {
      bool dirty;

      std::string filename;
      bool onDisk;

      GCode::ToolTable tools;
      Workpiece workpiece;
      Files files;

      GCode::Units units;

      ResolutionMode resolutionMode;
      double resolution;

    public:
      Project(const std::string &filename = std::string());
      ~Project();

      bool isDirty() const {return dirty;}
      void markDirty() {dirty = true;}
      void markClean() {dirty = false;}

      const std::string &getFilename() const {return filename;}
      void setFilename(const std::string &filename);
      std::string getDirectory() const;
      std::string getUploadFilename() const;

      bool isOnDisk() const {return onDisk;}

      const GCode::ToolTable &getTools() const {return tools;}
      GCode::ToolTable &getTools() {return tools;}

      const Workpiece &getWorkpiece() const {return workpiece;}
      Workpiece &getWorkpiece() {return workpiece;}

      unsigned getFileCount() const {return files.size();}
      const cb::SmartPointer<File> &getFile(unsigned i) const;
      std::string getFileRelativePath(unsigned i) const;
      cb::SmartPointer<File> findFile(const std::string &path) const;
      void addFile(const std::string &filename);
      void removeFile(unsigned i);

      void setUnits(GCode::Units units);
      GCode::Units getUnits() const {return units;}
      bool isMetric() const {return units == GCode::Units::METRIC;}

      ResolutionMode getResolutionMode() const {return resolutionMode;}
      void setResolutionMode(ResolutionMode mode);
      double getResolution() const;
      void setResolution(double resolution);
      static double computeResolution(ResolutionMode mode,
                                      cb::Rectangle3D bounds);

      void load(const std::string &filename);
      void save(const std::string &filename = std::string());

      // From JSON::Serializable
      void read(const cb::JSON::Value &value);
      void write(cb::JSON::Sink &sink) const;
    };
  }
}
