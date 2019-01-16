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

#include "ToolGraphicsItem.h"

#include <cbang/log/Logger.h>

using namespace CAMotics;


ToolGraphicsItem::ToolGraphicsItem(QGraphicsItem *parent) :
  QGraphicsPixmapItem(parent), number(0) {
}


void ToolGraphicsItem::update(const GCode::Tool &tool, const QSize &size) {
  number = tool.getNumber();

  // Update tool view
  toolView.setTool(tool);
  toolView.resize(size.width(), size.height());
  toolView.draw();

  // Paint image
  int stride = toolView.getStride();
  unsigned char *data = toolView.getBuffer().get();
  QImage image(data, size.width(), size.height(), stride,
               QImage::Format_ARGB32);

  setPixmap(QPixmap::fromImage(image));
}
