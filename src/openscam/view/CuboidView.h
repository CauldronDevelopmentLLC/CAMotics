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

#ifndef OPENSCAM_CUBOID_VIEW_H
#define OPENSCAM_CUBOID_VIEW_H

#include <openscam/Geom.h>


namespace OpenSCAM {
  class CuboidView {
    Rectangle3R bounds;

    unsigned vertexVBuf;
    unsigned normalVBuf;

  public:
    CuboidView(const Rectangle3R &bounds = Rectangle3R()) :
      bounds(bounds), vertexVBuf(0), normalVBuf(0) {}
    ~CuboidView();

    const Rectangle3R &getBounds() const {return bounds;}
    void setBounds(const Rectangle3R &bounds) {this->bounds = bounds;}

    void draw();
  };
}

#endif // OPENSCAM_CUBOID_VIEW_H

