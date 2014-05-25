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

#ifndef OPENSCAM_NCFILE_H
#define OPENSCAM_NCFILE_H

#include <cbang/StdTypes.h>
#include <cbang/SmartPointer.h>

#include <string>


namespace OpenSCAM {
  class Project;

  class NCFile {
    Project &project;
    std::string absPath;
    uint64_t modTime;

  public:
    NCFile(Project &project, const std::string &filename);

    bool isTPL() const;
    void setFilename(const std::string &filename);
    const std::string &getAbsolutePath() const {return absPath;}
    std::string getRelativePath() const;
    bool exists() const;
    uint64_t getTime() const;
    bool changed() const {return modTime < getTime();}
    void update() {modTime = getTime();}
  };
}

#endif // OPENSCAM_NCFILE_H

