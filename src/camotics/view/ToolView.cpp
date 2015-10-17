/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
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

#include "ToolView.h"

#include <cbang/String.h>

#include <cairo/cairo.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolView::~ToolView() {}


void ToolView::drawGuide(cairo_t *cr, double width, double x, double y,
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
  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  string s =
    String::printf("  %0.4f%s", value / scale,
                   tool.getUnits() == ToolUnits::UNITS_MM ? "mm" : "in");
  cairo_show_text(cr, s.c_str());

  // Connector
  cairo_rel_move_to(cr, 2, -3);
  cairo_line_to(cr, x, y + 12);
  cairo_rel_line_to(cr, 0, -8);
  cairo_stroke(cr);
}


void ToolView::resize(int width, int height) {
  if (this->width == width && this->height == height) return;

  stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  buffer = new unsigned char[stride * height];
  this->width = width;
  this->height = height;
}


void ToolView::draw() {
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
  ToolShape shape = tool.getShape();
  double length = tool.getLength();
  double diameter = tool.getDiameter();
  double snubDiameter =
    shape == ToolShape::TS_SNUBNOSE ? tool.getSnubDiameter() : 0;
  string title = String::printf("#%d", tool.getNumber());
  if (!simple) title = "Tool " + title + ": " + tool.getText();

  // Margin
  int margin = simple ? 12 : 50;
  cairo_save(cr);
  cairo_translate(cr, margin, margin);
  double w = width - 2 * margin;
  double h = height - (simple ? 1 : 2) * margin - (simple ? 1 : 0);

  // Title
  cairo_move_to(cr, w / 2, simple ? -2 : -20);
  cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, simple ? 12 : 16);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, title.c_str(), &extents);
  cairo_rel_move_to(cr, -extents.width / 2, 0);
  cairo_show_text(cr, title.c_str());

  // Scale
  double maxDim = diameter;
  if (shape == ToolShape::TS_SNUBNOSE && diameter < snubDiameter)
    maxDim = snubDiameter;
  if (maxDim < length) maxDim = length;
  double scale = h / maxDim;
  cairo_save(cr);
  cairo_scale(cr, scale, scale);
  w /= scale;
  h /= scale;

  // Cutter
  double x = (w - diameter) / 2.0;
  double y = (h - length) / 2.0;
  switch (shape) {
  case ToolShape::TS_CYLINDRICAL:
    cairo_rectangle(cr, x, y, diameter, length);
    break;

  case ToolShape::TS_SNUBNOSE:
  case ToolShape::TS_CONICAL:
    cairo_move_to(cr, x, y);
    if (diameter) cairo_rel_line_to(cr, diameter, 0);
    cairo_rel_line_to(cr, (snubDiameter - diameter) / 2, length);
    if (snubDiameter) cairo_rel_line_to(cr, -snubDiameter, 0);
    cairo_close_path(cr);
    break;

  case ToolShape::TS_BALLNOSE: {
    double radius = diameter / 2.0;
    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, diameter, 0);
    cairo_arc(cr, w / 2.0, y + length - radius, radius, 0, M_PI);
    cairo_close_path(cr);
    break;
  }

  case ToolShape::TS_SPHEROID:
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
    if (shape == ToolShape::TS_SNUBNOSE)
      drawGuide(cr, snubDiameter * scale, x, y + 15, "Snub Diameter",
                snubDiameter);

    cairo_rotate(cr, M_PI / 2.0);
    drawGuide(cr, length * scale, h / 2.0, -20, "Length", length);
  }

  // Cleanup
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}
