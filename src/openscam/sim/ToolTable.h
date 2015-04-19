/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
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

#ifndef OPENSCAM_TOOL_TABLE_H
#define OPENSCAM_TOOL_TABLE_H

#include "Tool.h"

#include <openscam/gcode/Processor.h>

#include <cbang/SmartPointer.h>
#include <cbang/xml/XMLHandlerFactory.h>
#include <cbang/xml/XMLHandler.h>

#include <map>

namespace cb {class XMLWriter;}


namespace OpenSCAM {
  class ToolTable :
    public std::map<unsigned, cb::SmartPointer<Tool> >, public Processor,
    public cb::XMLHandlerFactory, public cb::XMLHandler {

    cb::SmartPointer<Tool> current; ///< Used during XML parsing

  public:
    ToolTable();
    ToolTable(const ToolTable &o);

    bool has(unsigned tool) const;
    const cb::SmartPointer<Tool> &get(unsigned tool);
    void add(cb::SmartPointer<Tool> tool);
    void clear();

    // From Processor
    void operator()(const cb::SmartPointer<Block> &block);

    // From XMLHandlerFactory
    XMLHandler *getHandler(cb::XMLProcessor &processor,
                           const cb::XMLAttributes &attrs);
    void freeHandler(cb::XMLProcessor &processor, cb::XMLHandler *handler);

    // From XMLHandler
    void startElement(const std::string &name, const cb::XMLAttributes &attrs);
    void endElement(const std::string &name);
    void text(const std::string &text);

    void write(cb::XMLWriter &writer) const;
  };
}

#endif // OPENSCAM_TOOL_TABLE_H

