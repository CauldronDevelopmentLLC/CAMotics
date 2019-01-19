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

#include "Tool2DView.h"

#include <cbang/String.h>

#include <cairo/cairo.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


Tool2DView::~Tool2DView() {}


void Tool2DView::drawGuide(cairo_t *cr, double width, double x, double y,
                           const char *text, double value) {
  // Measure
  cairo_set_line_width(cr, 1);
  cairo_move_to(cr, x - width / 2.0, y);
  cairo_rel_line_to(cr, 0, 3);
  cairo_rel_line_to(cr, width, 0);
  cairo_rel_line_to(cr, 0, -3);
  cairo_stroke(cr);

  // Text
  cairo_move_to(cr, 5, y + 15);
  cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 12);
  cairo_show_text(cr, text);

  // Value
  double scale = tool.getUnits() == GCode::Units::METRIC ? 1.0 : 25.4;
  string s = "  " + String(value / scale) +
    (tool.getUnits() == GCode::Units::METRIC ? "mm" : "in");
  cairo_show_text(cr, s.c_str());

  // Connector
  cairo_rel_move_to(cr, 2, -3);
  cairo_line_to(cr, x, y + 12);
  cairo_rel_line_to(cr, 0, -8);
  cairo_stroke(cr);
}


void Tool2DView::drawText(cairo_t *cr, const string &text, unsigned fontSize,
                          bool bold) {
  cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL,
                         bold ? CAIRO_FONT_WEIGHT_BOLD :
                         CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, fontSize);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text.c_str(), &extents);
  cairo_rel_move_to(cr, -extents.width / 2, 0);
  cairo_show_text(cr, text.c_str());
}


void Tool2DView::resize(int width, int height) {
  if (this->width == width && this->height == height) return;

  stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  buffer = new unsigned char[stride * height];
  this->width = width;
  this->height = height;
}


void Tool2DView::draw() {
  cairo_surface_t *surface = cairo_image_surface_create_for_data
    (buffer.get(), CAIRO_FORMAT_ARGB32, width, height, stride);
  if (!surface) THROW("Failed to create cairo surface");

  cairo_t *cr = cairo_create(surface);

  bool simple = width < 300 || height < 300;

  // Background & foreground
  const double bg = 1.0;
  const double fg = 0.2;

  cairo_set_source_rgba(cr, bg, bg, bg, 1);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, fg, fg, fg, 1);

  // Get tool info
  GCode::ToolShape shape = tool.getShape();
  double length = tool.getLength();
  double diameter = tool.getDiameter();
  double snubDiameter =
    shape == GCode::ToolShape::TS_SNUBNOSE ? tool.getSnubDiameter() : 0;
  string title = String::printf("Tool #%d", tool.getNumber());
  if (!simple) title = title + ": " + tool.getText();

  // Margin
  int margin = simple ? 14 : 50;
  cairo_save(cr);
  cairo_translate(cr, margin, margin);
  double w = width - 2 * margin;
  double h = height - 2 * margin;

  // Title
  cairo_move_to(cr, w / 2, simple ? -2 : -20);
  drawText(cr, title, simple ? 12 : 16, true);

  // Subtitle
  if (simple) {
    cairo_move_to(cr, w / 2, h + 10);
    drawText(cr, tool.getSizeText(), 10, true);
  }

  // Scale
  double xDim =
    (shape == GCode::ToolShape::TS_SNUBNOSE && diameter < snubDiameter) ?
    snubDiameter : diameter;
  double yDim = length;
  double xScale = w / xDim;
  double yScale = h / yDim;
  double scale = xScale < yScale ? xScale : yScale;

  cairo_save(cr);
  cairo_scale(cr, scale, scale);
  w /= scale;
  h /= scale;

  // Cutter
  double x = (w - diameter) / 2.0;
  double y = (h - length) / 2.0;
  switch (shape) {
  case GCode::ToolShape::TS_CYLINDRICAL:
    cairo_rectangle(cr, x, y, diameter, length);
    break;

  case GCode::ToolShape::TS_SNUBNOSE:
  case GCode::ToolShape::TS_CONICAL:
    cairo_move_to(cr, x, y);
    if (diameter) cairo_rel_line_to(cr, diameter, 0);
    cairo_rel_line_to(cr, (snubDiameter - diameter) / 2, length);
    if (snubDiameter) cairo_rel_line_to(cr, -snubDiameter, 0);
    cairo_close_path(cr);
    break;

  case GCode::ToolShape::TS_BALLNOSE: {
    double radius = diameter / 2.0;
    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, diameter, 0);
    cairo_arc(cr, w / 2.0, y + length - radius, radius, 0, M_PI);
    cairo_close_path(cr);
    break;
  }

  case GCode::ToolShape::TS_SPHEROID:
    cairo_save(cr);
    cairo_translate(cr, w / 2.0, y + length / 2.0);
    cairo_scale(cr, diameter / 2.0, length / 2.0);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(cr);
    break;
  }

  cairo_fill(cr);

  cairo_restore(cr);
  w *= scale;
  h *= scale;

  // Guides
  if (!simple) {
    x = w / 2.0;
    y = h + 5;
    drawGuide(cr, diameter * scale, x, y, "Diameter", diameter);
    if (shape == GCode::ToolShape::TS_SNUBNOSE)
      drawGuide(cr, snubDiameter * scale, x, y + 15, "Snub Diameter",
                snubDiameter);

    cairo_rotate(cr, M_PI / 2.0);
    drawGuide(cr, length * scale, h / 2.0, 3, "Length", length);
  }

  // Cleanup
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}
