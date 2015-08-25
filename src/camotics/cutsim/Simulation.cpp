/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#include "Simulation.h"
#include "ToolPath.h"
#include "Workpiece.h"

#include <camotics/sim/ToolTable.h>

#include <cbang/json/JSON.h>
#include <cbang/openssl/DigestStreamFilter.h>

#include <boost/ref.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/filtering_stream.hpp>
namespace io = boost::iostreams;

using namespace std;
using namespace cb;
using namespace CAMotics;


string Simulation::computeHash() const {
  DigestStreamFilter digest("sha256");

  io::filtering_ostream stream;
  stream.push(boost::ref(digest));
  stream.push(io::null_sink());

  JSON::Writer writer(stream);
  write(writer);

  stream.reset();

  return digest.toBase64();
}


void Simulation::write(JSON::Sink &sink) const {
  sink.beginDict();

  sink.beginInsert("tools");
  tools->write(sink);

  sink.beginInsert("path");
  path->write(sink);

  sink.beginInsert("workpiece");
  workpiece->write(sink);

  sink.insert("resolution", resolution);
  sink.insert("time", time);

  sink.endDict();
}
