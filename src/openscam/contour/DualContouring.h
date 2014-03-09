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

#ifndef OPENSCAM_DUAL_CONTOURING_H
#define OPENSCAM_DUAL_CONTOURING_H

#include "ContourGenerator.h"
#include "ElementSurface.h"

#include <openscam/Geom.h>


namespace OpenSCAM {
  class DualContouring : public ContourGenerator {
    cb::SmartPointer<ElementSurface> surface;

  public:
    // From ContourGenerator
    cb::SmartPointer<Surface> getSurface() {return surface;}
    void run(FieldFunction &func, const Rectangle3R &bbox, real step);
  };
}

#endif // OPENSCAM_DUAL_CONTOURING_H

