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

#include "ToolGraphicsItem.h"

#include <cbang/log/Logger.h>

using namespace CAMotics;


ToolGraphicsItem::ToolGraphicsItem(QGraphicsItem *parent) :
  QGraphicsPixmapItem(parent), number(0), highlight(false) {
  setAcceptHoverEvents(true);
  setCursor(Qt::OpenHandCursor);
  setAcceptedMouseButtons(Qt::LeftButton);
}


void ToolGraphicsItem::update(const Tool &tool, int width, int height) {
  number = tool.getNumber();

  // Update tool view
  toolView.setTool(tool);
  toolView.resize(width, height);
  toolView.draw();

  // Paint image
  int stride = toolView.getStride();
  unsigned char *data = toolView.getBuffer().get();
  QImage image(data, width, height, stride, QImage::Format_ARGB32);

  setPixmap(QPixmap::fromImage(image));
}


void ToolGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
  highlight = true;
  QGraphicsPixmapItem::update();
}


void ToolGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
  highlight = false;
  QGraphicsPixmapItem::update();
}


void ToolGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ClosedHandCursor);
}


void ToolGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::OpenHandCursor);
}


void ToolGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
#if 0
  if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
      .length() < QApplication::startDragDistance()) {
    return;
  }

  QDrag *drag = new QDrag(event->widget());
  QMimeData *mime = new QMimeData;
  mime->setData("application/x-camotics-tool", QByteArray());
  drag->setMimeData(mime);
  drag->setPixmap(pixmap());
  drag->setHotSpot(QPoint(pixmap().width() / 2, pixmap().height() / 2));
  drag->exec();

  setCursor(Qt::OpenHandCursor);
#endif
}


void ToolGraphicsItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget) {
  QGraphicsPixmapItem::paint(painter, option, widget);

  if (highlight) {
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());
  }
}
