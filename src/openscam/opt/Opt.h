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

#ifndef OPENSCAM_OPT_H
#define OPENSCAM_OPT_H

#include "Group.h"

#include <openscam/Geom.h>
#include <openscam/gcode/Printer.h>

#include <openscam/sim/Machine.h>
#include <openscam/gcode/Interpreter.h>

#include <cbang/SmartPointer.h>
#include <cbang/io/Reader.h>

#include <string>
#include <istream>


namespace cb {class Options;}

namespace OpenSCAM {
  class AnnealState;

  class Opt : public Machine, public Printer, public cb::Reader {
    Controller controller;
    Interpreter interp;

    unsigned pathCount;
    unsigned cutCount;

    typedef std::vector<cb::SmartPointer<Group> > groups_t;
    groups_t groups;

    cb::SmartPointer<Path> currentPath;
    cb::SmartPointer<Group> currentGroup;

    unsigned iterations;
    unsigned runs;
    double heatTarget;
    double minTemp;
    double heatRate;
    double coolRate;
    double reheatRate;
    unsigned timeout;

  public:
    Opt(cb::Options &options, std::ostream &stream);

    double computeCost() const;

    // From cb::Reader
    void read(const cb::InputSource &source);

    // From Machine
    void move(const Move &move);

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

  protected:
    double round(double T, unsigned iterations, AnnealState &current,
                 AnnealState &best);
    double optimize(Group &group);
  };
}

#endif // OPENSCAM_OPT_H

