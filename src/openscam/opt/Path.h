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

#ifndef OPENSCAM_PATH_H
#define OPENSCAM_PATH_H

#include <openscam/cutsim/Move.h>

#include <cbang/geom/Vector.h>

#include <list>

namespace OpenSCAM {
  class Path : public std::list<Move> {
  public:

    const Vector3R &startPoint() const {return front()[0];}
    const Vector3R &endPoint() const {return back()[1];}
    void reverse();
    double costTo(const Path &o) const
    {return endPoint().distance(o.startPoint());}
  };
}

#endif // OPENSCAM_PATH_H

