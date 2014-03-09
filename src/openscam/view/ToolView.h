/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_TOOL_VIEW_H
#define OPENSCAM_TOOL_VIEW_H

#include <cbang/SmartPointer.h>

typedef struct _cairo cairo_t;


namespace OpenSCAM {
  class Tool;

  class ToolView {
    cb::SmartPointer<Tool> tool;
    cb::SmartPointer<unsigned char>::Array buffer;

    int width;
    int height;
    int stride;

  public:
    ToolView() : width(0), height(0), stride(0) {}
    ~ToolView();

    void setTool(const cb::SmartPointer<Tool> &tool) {this->tool = tool;}

    const cb::SmartPointer<unsigned char>::Array &getBuffer() const
    {return buffer;}
    int getWidth() const {return width;}
    int getHeight() const {return height;}
    int getStride() const {return stride;}

    void drawGuide(cairo_t *cr, double width, double x, double y,
                   const char *text, double value);

    void resize(int w, int h);
    void draw();
  };
}

#endif // OPENSCAM_TOOL_VIEW_H

