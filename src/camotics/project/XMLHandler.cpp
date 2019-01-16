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

#include "XMLHandler.h"


using namespace CAMotics::Project;
using namespace std;
using cb::String;


namespace {
  string decodeFilename(const string &filename) {
    string result;

    for (unsigned i = 0; i < filename.size(); i++)
      if (filename[i] == '%' && i < filename.size() - 2) {
        result += (char)String::parseU8("0x" + filename.substr(i + 1, 2));
        i += 2;

      } else result += filename[i];

    return result;
  }
}


XMLHandler::XMLHandler(cb::JSON::Sink &sink) :
  sink(sink), inTools(false), tools(0), currentTool(-1),
  inFiles(false), automaticWorkpiece(true), workpieceMargin(5) {}


void XMLHandler::pushFile(const string &filename) {sink.beginDict();}


void XMLHandler::popFile() {
  // Workpiece
  sink.insertDict("workpiece");

  sink.insertBoolean("automatic", automaticWorkpiece);
  sink.beginInsert("bounds");
  wpBounds.write(sink);
  sink.insert("margin", workpieceMargin);

  sink.endDict();

  sink.endDict();
}


void XMLHandler::startElement(const string &name,
                              const cb::XMLAttributes &attrs) {
  if (inTools) {
    if (name == "tool") {
      unsigned number;
      if (attrs.has("number")) number = String::parseU32(attrs["number"]);
      else number = tools;

      sink.insertDict(String(number));

      if (attrs.has("units")) sink.insert("units", attrs["units"]);
      if (attrs.has("shape")) sink.insert("shape", attrs["shape"]);

      if (attrs.has("length"))
        sink.insert("length", String::parseDouble(attrs["length"]));

      if (attrs.has("radius"))
        sink.insert("diameter", String::parseDouble(attrs["radius"]) * 2);
      else if (attrs.has("diameter"))
        sink.insert("diameter", String::parseDouble(attrs["diameter"]));

      if (attrs.has("snub_diameter"))
        sink.insert("snub_diameter",
                    String::parseDouble(attrs["snub_diameter"]));
    }

  } else if (inFiles) ; // Ignore any XML elements

  else if (name == "tool_table") {
    inTools = true;
    sink.insertDict("tools");

  } else if (name == "nc-files") {
    inFiles = true;
    sink.insertList("files");

  } else if (attrs.has("v")) {
    if (name == "units")
      sink.insert("units", String::toLower(attrs["v"]) == "inch" ?
                  "imperial" : "metric");

    else if (name == "resolution-mode")
      sink.insert("resolution-mode", attrs["v"]);

    else if (name == "resolution")
      sink.insert("resolution", String::parseDouble(attrs["v"]));

    else if (name == "automatic-workpiece")
      automaticWorkpiece = String::parseBool(attrs["v"]);

    else if (name == "workpiece-min") wpBounds.setMin(cb::Vector3D(attrs["v"]));
    else if (name == "workpiece-max") wpBounds.setMax(cb::Vector3D(attrs["v"]));

    else if (name == "workpiece-margin")
      workpieceMargin = String::parseDouble(attrs["v"]);
  }
}


void XMLHandler::endElement(const string &name) {
  if (name == "tool_table") {
    inTools = false;
    sink.endDict();

  } else if (name == "nc-files") {
    inFiles = false;
    sink.endList();
  }

  if (inTools && name == "tool") sink.endDict();

  currentTool = -1;
}


void XMLHandler::text(const string &text) {
  if (0 <= currentTool) {
    string desc = String::trim(text);
    if (!desc.empty()) sink.insert("description", desc);
  }

  if (inFiles) {
    vector<string> files;
    String::tokenize(text, files);

    for (unsigned i = 0; i < files.size(); i++)
      sink.append(decodeFilename(files[i]));
  }
}
