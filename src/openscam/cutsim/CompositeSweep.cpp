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

#include "CompositeSweep.h"

using namespace std;
using namespace cb;
using namespace OpenSCAM;


void CompositeSweep::add(const SmartPointer<Sweep> &sweep, real zOffset) {
  children.push_back(sweep);
  zOffsets.push_back(zOffset);
}


void CompositeSweep::getBBoxes(const Vector3R &start, const Vector3R &end,
                               vector<Rectangle3R> &bboxes,
                               real tolerance) const {
  for (unsigned i = 0; i < children.size(); i++)
    children[i]->getBBoxes(start, end, bboxes, tolerance);
}


bool CompositeSweep::contains(const Vector3R &start, const Vector3R &end,
                              const Vector3R &p) const {
  for (unsigned i = 0; i < children.size(); i++)
    if (children[i]->contains(start, end, p - Vector3R(0, 0, zOffsets[i])))
      return true;

  return false;
}
