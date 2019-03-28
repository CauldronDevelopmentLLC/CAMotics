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

#include "ToolTable.h"

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/log/Logger.h>
#include <cbang/json/Sink.h>
#include <cbang/json/Dict.h>

using namespace std;
using namespace cb;
using namespace GCode;


bool ToolTable::has(unsigned tool) const {return find(tool) != end();}


const Tool &ToolTable::at(unsigned index) const {
  for (const_iterator it = begin(); it != end(); it++)
    if (!index--) return it->second;

  THROW("No tool at index " << index);
}


const Tool &ToolTable::get(unsigned tool) const {
  const_iterator it = find(tool);
  if (it != end()) return it->second;

  THROW("Missing tool " << tool);
}


Tool &ToolTable::get(unsigned tool) {
  iterator it = find(tool);
  if (it != end()) return it->second;

  LOG_WARNING("Auto-creating missing tool " << tool);
  return (*this)[tool] = Tool(tool);
}


void ToolTable::set(const Tool &tool) {
  insert(value_type(tool.getNumber(), tool)).first->second = tool;
}


void ToolTable::add(const Tool &tool) {
  if (!insert(value_type(tool.getNumber(), tool)).second)
    THROW("Tool with number " << tool.getNumber()
           << " already in tool table");

  LOG_INFO(3, "Added tool " << tool.getNumber() << " with radius "
           << tool.getRadius());
}


void ToolTable::read(const JSON::Value &value) {
  clear();

  for (unsigned i = 0; i < value.size(); i++) {
    string key = value.keyAt(i);
    unsigned number = String::parseU32(key);
    Tool tool(number);

    tool.read(value.getDict(i));
    set(tool);
  }
}


void ToolTable::write(JSON::Sink &sink) const {
  sink.beginDict();

  for (const_iterator it = begin(); it != end(); it++) {
    sink.beginInsert(String(it->second.getNumber()));
    it->second.write(sink, false);
  }

  sink.endDict();
}
