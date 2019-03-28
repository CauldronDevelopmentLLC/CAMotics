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

#include "ProbeGrid.h"

#include <cbang/Exception.h>
#include <cbang/Math.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ProbeGrid::ProbeGrid(const Rectangle2D &bbox, const Vector2D &divisions) :
  bbox(bbox), divisions(divisions),
  cellSize(bbox.getWidth() / divisions.x(), bbox.getLength() / divisions.y()) {

  // Resize vectors
  resize(divisions.y() + 1, Column_t(divisions.x() + 1));

  // Fill in coords
  double y = bbox.getMin().y();
  for (ProbeGrid::row_iterator row = begin(); row != end(); row++) {
    double x = bbox.getMin().x();

    for (ProbeGrid::col_iterator col = row->begin(); col != row->end(); col++) {
      col->x() = x;
      col->y() = y;
      x += cellSize.x();
    }

    y += cellSize.y();
  }
}


vector<ProbePoint *> ProbeGrid::find(const Vector2D &p) {
  double xA = (p.x() - bbox.getMin().x()) / cellSize.x();
  double yA = (p.y() - bbox.getMin().y()) / cellSize.y();
  int x = floor(xA);
  int y = floor(yA);

  if (y < 0 || (int)size() <= y || x < 0 || (int)(*this)[y].size() <= x)
    THROW("Point " << p << " not in grid (" << x << ", " << y << ')'
           << " (" << (*this)[y].size() << ", " << size() << ')');

  if (y == yA && y == (int)size() - 1) y--;
  if (x == xA && x == (int)(*this)[y].size() - 1) x--;

  vector<ProbePoint *> points;
  points.push_back(&(*this)[y][x]);
  points.push_back(&(*this)[y][x + 1]);
  points.push_back(&(*this)[y + 1][x]);
  points.push_back(&(*this)[y + 1][x + 1]);

  return points;
}
