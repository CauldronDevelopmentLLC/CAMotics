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

#pragma once


#include "Tool.h"

#include <gcode/Processor.h>

#include <cbang/xml/XMLHandlerFactory.h>
#include <cbang/xml/XMLHandler.h>

#include <map>


namespace GCode {
  class ToolTable :
    public std::map<unsigned, Tool>, public Processor,
    public cb::XMLHandlerFactory, public cb::XMLHandler,
    public cb::JSON::Serializable {

    int current; ///< Used during XML parsing

  public:
    ToolTable();

    bool has(unsigned tool) const;
    const Tool &at(unsigned index) const;
    const Tool &get(unsigned tool) const;
    Tool &get(unsigned tool);
    void set(const Tool &tool);
    void add(const Tool &tool);

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

    // From JSON::Serializable
    using cb::JSON::Serializable::read;
    using cb::JSON::Serializable::write;
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;
  };
}
