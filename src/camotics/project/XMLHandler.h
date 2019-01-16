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

#pragma once

#include <cbang/xml/XMLHandler.h>
#include <cbang/geom/Rectangle.h>
#include <cbang/json/Sink.h>


namespace CAMotics {
  namespace Project {
    class XMLHandler : public cb::XMLHandler {
      cb::JSON::Sink &sink;

      bool inTools;
      unsigned tools;
      int currentTool;

      bool inFiles;

      bool automaticWorkpiece;
      cb::Rectangle3D wpBounds;
      double workpieceMargin;

    public:
      XMLHandler(cb::JSON::Sink &sink);

      // From XMLHandler
      void pushFile(const std::string &filename);
      void popFile();
      void startElement(const std::string &name,
                        const cb::XMLAttributes &attrs);
      void endElement(const std::string &name);
      void text(const std::string &text);
    };
  }
}
