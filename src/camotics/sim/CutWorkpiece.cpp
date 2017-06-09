/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "CutWorkpiece.h"

#include <cbang/Math.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


CutWorkpiece::CutWorkpiece(const SmartPointer<ToolSweep> &toolSweep,
                           const Workpiece &workpiece) :
  toolSweep(toolSweep), workpiece(workpiece) {}


bool CutWorkpiece::isValid() const {
  if (workpiece.isValid()) return false;

  cb::Rectangle3D bounds = getBounds();
  for (unsigned i = 0; i < 3; i++)
    if (Math::isnan(bounds.getMin()[i]) || Math::isnan(bounds.getMax()[i]) ||
        Math::isinf(bounds.getMin()[i]) || Math::isinf(bounds.getMax()[i]))
      return false;

  return true;
}


cb::Rectangle3D CutWorkpiece::getBounds() const {
  cb::Rectangle3D bb;
  if (workpiece.isValid()) bb = workpiece.getBounds();
  else if (!toolSweep.isNull()) bb = toolSweep->getBounds();
  return bb;
}


bool CutWorkpiece::cull(const cb::Rectangle3D &r) const {
  return toolSweep->cull(r);
}


double CutWorkpiece::depth(const cb::Vector3D &p) const {
  if (!workpiece.isValid()) return toolSweep->depth(p);
  return min(workpiece.depth(p), -toolSweep->depth(p));
}
