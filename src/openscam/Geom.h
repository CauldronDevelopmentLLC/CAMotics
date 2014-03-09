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

#ifndef OPENSCAM_GEOM_H
#define OPENSCAM_GEOM_H

#include "Real.h"

#include <cbang/geom/Vector.h>
#include <cbang/geom/Segment.h>
#include <cbang/geom/Triangle.h>
#include <cbang/geom/Rectangle.h>
#include <cbang/geom/Path.h>
#include <cbang/geom/Quaternion.h>
#include <cbang/geom/AxisAngle.h>


namespace OpenSCAM {
  typedef cb::Vector<2, real> Vector2R;
  typedef cb::Vector<3, real> Vector3R;
  typedef cb::Segment<2, real> Segment2R;
  typedef cb::Segment<3, real> Segment3R;
  typedef cb::Vector<3, Vector2R> Triangle2R;
  typedef cb::Vector<3, Vector3R> Triangle3R;
  typedef cb::Rectangle<2, real> Rectangle2R;
  typedef cb::Rectangle<3, real> Rectangle3R;
  typedef cb::Path<2, real> Path2R;
  typedef cb::Path<3, real> Path3R;
  typedef cb::Quaternion<real> Quaternion;
  typedef cb::AxisAngle<real> AxisAngle;
}

#endif // OPENSCAM_GEOM_H

