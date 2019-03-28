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

#include "Transforms.h"

#include <cbang/Exception.h>

using namespace GCode;
using namespace cb;


TransformStack &Transforms::get(axes_t axes) {
  if (AXES_COUNT <= axes) THROW("Invalid transform " << axes);
  return stacks[axes];
}


const TransformStack &Transforms::get(axes_t axes) const {
  if (AXES_COUNT <= axes) THROW("Invalid transform " << axes);
  return stacks[axes];
}


Axes Transforms::transform(const Axes &axes) const {
  Axes trans(axes);

  // TODO this is inefficient
  trans.applyXYZMatrix(get(XYZ).top());
  trans.applyABCMatrix(get(ABC).top());
  trans.applyUVWMatrix(get(UVW).top());

  return trans;
}
