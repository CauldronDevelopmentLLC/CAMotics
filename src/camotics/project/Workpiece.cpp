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

#include "Workpiece.h"

#include <camotics/sim/Sweep.h>
#include <camotics/sim/ToolSweep.h>

#include <gcode/ToolPath.h>
#include <gcode/Move.h>

#include <cbang/json/Dict.h>

using namespace CAMotics::Project;
using namespace cb;
using namespace std;


void Workpiece::setBounds(const Rectangle3D &bounds) {this->bounds = bounds;}


void Workpiece::update(GCode::ToolPath &path) {
  if (!isAutomatic()) return;
  Rectangle3D bounds;

  // Guess workpiece bounds from cutting moves
  vector<SmartPointer<Sweep> > sweeps;
  vector<Rectangle3D> bboxes;

  for (unsigned i = 0; i < path.size(); i++) {
    const GCode::Move &move = path.at(i);

    if (move.getType() == GCode::MoveType::MOVE_RAPID) continue;

    int tool = move.getTool();
    if (tool < 0) continue;

    if (sweeps.size() <= (unsigned)tool) sweeps.resize(tool + 1);
    if (sweeps[tool].isNull())
      sweeps[tool] = ToolSweep::getSweep(path.getTools().get(tool));

    sweeps[tool]->getBBoxes(move.getStartPt(), move.getEndPt(), bboxes, 0);
  }

  for (unsigned i = 0; i < bboxes.size(); i++) bounds.add(bboxes[i]);

  if (bounds == Rectangle3D()) return;

  // Compute auto workpiece bounds
  Vector3D bMin = bounds.getMin();
  Vector3D bMax = bounds.getMax();
  double zMax = 0 <= bMin.z() ? bMax.z() : 0;
  bounds = Rectangle3D(bMin, Vector3D(bMax.x(), bMax.y(), zMax));

  // At least 2mm thick
  if (bounds.getHeight() < 2)
    bounds.add(Vector3D(bMin.x(), bMin.y(), bMin.z() - 2));

  if (bounds.isReal()) {
    // Add margin
    Vector3D margin = bounds.getDimensions() * getMargin() / 100.0;
    bounds.add(bounds.getMin() - margin);
    bounds.add(bounds.getMax() + Vector3D(margin.x(), margin.y(), 0));

    setBounds(bounds);
  }
}


void Workpiece::read(const JSON::Value &value) {
  automatic = value.getBoolean("automatic", automatic);
  margin = value.getNumber("margin", margin);
  if (value.hasDict("bounds")) bounds.read(value.getDict("bounds"));
}


void Workpiece::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insertBoolean("automatic", automatic);
  sink.insert("margin", margin);
  if (bounds.isValid()) {
    sink.beginInsert("bounds");
    bounds.write(sink);
  }

  sink.endDict();
}
