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

#include "STL.h"

#include "STLWriter.h"
#include "STLReader.h"

#include <camotics/Task.h>

#include <cbang/Exception.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


void STL::read(STLSource &source, Task *task) {
  if (task) task->begin();
  uint32_t count = source.readHeader(name, hash); // ASCII STL files return 0
  if (count) reserve(count);

  for (unsigned i = 0; source.hasMore(); i++) {
    add(source.readFacet());

    if (task) {
      if (!count && 10000 < i) i = 0;
      task->update((double)i / (count ? count : 10000), "Reading STL");
    }
  }

  source.readFooter();
  if (task) task->end();
}


void STL::write(STLSink &sink, Task *task) const {
  if (task) task->begin();
  sink.writeHeader(name, size(), hash);

  for (unsigned i = 0; i < size(); i++) {
    sink.writeFacet((*this)[i]);

    if (task) task->update((double)i / size(), "Writing STL");
  }

  sink.writeFooter(name, hash);
  if (task) task->end();
}


void STL::read(const InputSource &source, Task *task) {
  STLReader reader(source);
  read(reader, task);
  if (source.getStream().bad()) THROWS("Error while reading STL");
}


void STL::write(const OutputSink &sink, Task *task, bool binary) const {
  STLWriter writer(sink, binary);
  write(writer, task);
  if (sink.getStream().bad()) THROWS("Error while writing STL");
}
