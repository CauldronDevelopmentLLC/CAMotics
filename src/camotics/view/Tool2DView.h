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


#include <gcode/Tool.h>

#include <cbang/SmartPointer.h>

typedef struct _cairo cairo_t;


namespace GCode {class Tool;}

namespace CAMotics {
  class Tool2DView {
    GCode::Tool tool;
    cb::SmartPointer<unsigned char>::Array buffer;

    int width;
    int height;
    int stride;

  public:
    Tool2DView() : width(0), height(0), stride(0) {}
    ~Tool2DView();

    void setTool(const GCode::Tool &tool) {this->tool = tool;}

    const cb::SmartPointer<unsigned char>::Array &getBuffer() const
    {return buffer;}
    int getWidth() const {return width;}
    int getHeight() const {return height;}
    int getStride() const {return stride;}

    void drawGuide(cairo_t *cr, double width, double x, double y,
                   const char *text, double value);
    void drawText(cairo_t *cr, const std::string &text, unsigned fontSize,
                  bool bold);

    void resize(int w, int h);
    void draw();
  };
}
