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

#include "Simulation.h"
#include "Workpiece.h"

#include <camotics/SHA256.h>
#include <camotics/contour/TriangleSurface.h>

#include <cbang/json/JSON.h>
#include <cbang/iostream/UpdateStreamFilter.h>
#include <cbang/net/Base64.h>

#include <boost/ref.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/filtering_stream.hpp>
namespace io = boost::iostreams;

using namespace std;
using namespace cb;
using namespace CAMotics;


Simulation::~Simulation() {}


string Simulation::computeHash() const {
  SHA256 sha256;
  UpdateStreamFilter<SHA256> digest(sha256);

  io::filtering_ostream stream;
  stream.push(boost::ref(digest));
  stream.push(io::null_sink());

  JSON::Writer writer(stream);
  write(writer);

  stream.reset();

  return Base64().encode(sha256.finalize());
}


void Simulation::read(const JSON::Value &value) {
  resolution = value.getNumber("resolution", 0);
  time = value.getNumber("time", 0);
  mode = RenderMode::parse(value.getString("render-mode", mode.toString()));

  GCode::ToolTable tools;
  if (value.has("tools")) tools.read(*value.get("tools"));

  if (value.has("workpiece")) workpiece.read(*value.get("workpiece"));
  else workpiece = Rectangle3D();

  if (value.has("path")) {
    path = new GCode::ToolPath(tools);
    path->read(*value.get("path"));
  }

  if (value.has("surface")) {
    SmartPointer<TriangleSurface> surface = new TriangleSurface;
    surface->read(*value.get("surface"));
    this->surface = surface;
  }

  if (value.has("planner")) {
    planConf = new GCode::PlannerConfig;
    planConf->read(*value.get("planner"));
  }
}


void Simulation::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.insert("resolution", resolution);
  sink.insert("time", time);
  sink.insert("render-mode", mode.toString());

  if (!path.isNull() && !path->getTools().empty()) {
    sink.beginInsert("tools");
    path->getTools().write(sink);
  }

  if (workpiece != Rectangle3D()) {
    sink.beginInsert("workpiece");
    workpiece.write(sink);
  }

  if (path.isSet()) {
    sink.beginInsert("path");
    path->write(sink);
  }

  if (surface.isSet()) {
    sink.beginInsert("surface");
    surface->write(sink);
  }

  if (planConf.isSet()) {
    sink.beginInsert("planner");
    planConf->write(sink);
  }

  sink.endDict();
}
