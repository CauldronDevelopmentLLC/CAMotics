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

#include "CuboidView.h"

#include "Transform.h"

using namespace cb;
using namespace CAMotics;


CuboidView::CuboidView() : Mesh(12) {
  static float vertices[] = {
    1, 0, 0,  1, 0, 1,  1, 1, 0,   1, 0, 1,  1, 1, 1,  1, 1, 0,
    0, 0, 0,  0, 0, 1,  0, 1, 0,   0, 0, 1,  0, 1, 1,  0, 1, 0,

    0, 1, 0,  0, 1, 1,  1, 1, 0,   0, 1, 1,  1, 1, 1,  1, 1, 0,
    0, 0, 0,  0, 0, 1,  1, 0, 0,   0, 0, 1,  1, 0, 1,  1, 0, 0,

    0, 0, 1,  0, 1, 1,  1, 0, 1,   0, 1, 1,  1, 1, 1,  1, 0, 1,
    0, 0, 0,  0, 1, 0,  1, 0, 0,   0, 1, 0,  1, 1, 0,  1, 0, 0,
  };

  static float normals[] = {
     1,  0,  0,   1,  0,  0,   1,  0,  0,   1,  0,  0,   1,  0,  0,   1,  0,  0,
    -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  -1,  0,  0,

     0,  1,  0,   0,  1,  0,   0,  1,  0,   0,  1,  0,   0,  1,  0,   0,  1,  0,
     0, -1,  0,   0, -1,  0,   0, -1,  0,   0, -1,  0,   0, -1,  0,   0, -1,  0,

     0,  0,  1,   0,  0,  1,   0,  0,  1,   0,  0,  1,   0,  0,  1,   0,  0,  1,
     0,  0, -1,   0,  0, -1,   0,  0, -1,   0,  0, -1,   0,  0, -1,   0,  0, -1,
  };

  Mesh::add(36, vertices, normals);
}


void CuboidView::setBounds(const Rectangle3D &bounds) {
  Vector3D bMin = bounds.getMin();
  Vector3D bDim = bounds.getDimensions();

  getTransform().toIdentity();
  getTransform().translate(bMin.x(), bMin.y(), bMin.z());
  getTransform().scale(bDim.x(), bDim.y(), bDim.z());
}
