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

#include "SurfaceThread.h"

#include <camotics/stl/STLReader.h>
#include <camotics/contour/ElementSurface.h>

#include <cbang/util/DefaultCatch.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/iostream/BZip2Decompressor.h>

#include <boost/iostreams/filtering_stream.hpp>
namespace io = boost::iostreams;

using namespace std;
using namespace cb;
using namespace CAMotics;


void SurfaceThread::run() {
  // Check for cached STL file
  try {
    string stlCache = SystemUtilities::swapExtension(filename, "stl");
    bool hasCache = SystemUtilities::exists(stlCache);
    bool hasBZ2Cache = SystemUtilities::exists(stlCache + ".bz2");

    if (hasCache || hasBZ2Cache) {
      string filename = stlCache + (hasBZ2Cache ? ".bz2" : "");
      SmartPointer<istream> stream = SystemUtilities::iopen(filename);

      io::filtering_istream in;
      if (hasBZ2Cache) in.push(BZip2Decompressor());
      in.push(*stream);

      STLReader reader(in);

      string name;
      string hash;
      reader.readHeader(name, hash);

      if (hash == sim->computeHash()) {
        LOG_INFO(1, "Loading precomputed surface from " << filename);
        surface = new ElementSurface(reader, cutSim.get());
        reader.readFooter();
      }
    }
  } CATCH_ERROR;

  // Compute surface
  try {
    if (surface.isNull()) surface = cutSim->computeSurface(*sim);
  } CATCH_ERROR;

  completed();
}
