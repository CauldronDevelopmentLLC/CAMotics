/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#ifndef OPENSCAM_CUT_WORKPIECE_H
#define OPENSCAM_CUT_WORKPIECE_H

#include "Workpiece.h"
#include "ToolSweep.h"

#include <openscam/Geom.h>
#include <openscam/contour/FieldFunction.h>

#include <cbang/SmartPointer.h>


namespace OpenSCAM {
  class CutWorkpiece : public FieldFunction {
    cb::SmartPointer<ToolSweep> toolSweep;
    cb::SmartPointer<Workpiece> workpiece;

    uint64_t samples;

  public:
    CutWorkpiece(cb::SmartPointer<ToolSweep> toolSweep,
                 cb::SmartPointer<Workpiece> workpiece);

    cb::SmartPointer<ToolSweep> getToolSweep() {return toolSweep;}
    cb::SmartPointer<Workpiece> getWorkpiece() {return workpiece;}

    uint64_t getSampleCount() const {return samples;}
    void clearSampleCount() {samples = 0;}

    bool isValid() const;
    Rectangle3R getBounds() const;

    void drawBB() {toolSweep->drawBB();}

    // From FieldFunction
    bool contains(const Vector3R &p);
    bool canCull(const Rectangle3R &bbox);
  };
}

#endif // OPENSCAM_CUT_WORKPIECE_H
