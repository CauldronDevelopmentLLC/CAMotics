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

#include "ToolTableScene.h"
#include "ToolGraphicsItem.h"

#include <cbang/Math.h>
#include <cbang/log/Logger.h>

using namespace std;
using namespace CAMotics;


void ToolTableScene::update(const ToolTable &tools, QSize dims) {
  QGraphicsScene::clear();

  const unsigned toolWidth = 50;
  const unsigned toolHeight = 50;
  const unsigned margin = 4;

  const unsigned columns =
    (unsigned)floor((dims.width() - margin) / (double)(toolWidth + margin));
  const unsigned rows = (unsigned)ceil(tools.size() / (double)columns);

  const unsigned width = dims.width();
  const unsigned height = (toolHeight + margin) * rows + margin;
  QGraphicsScene::setSceneRect(0, 0, width, height);

  unsigned i = 0;
  unsigned x = margin;
  unsigned y = margin;

  for (ToolTable::const_iterator it = tools.begin(); it != tools.end(); it++) {
    const Tool &tool = it->second;
    if (!tool.getNumber()) continue;

    // Tool
    ToolGraphicsItem *item = new ToolGraphicsItem;
    item->update(tool, toolWidth, toolHeight);
    item->setPos(x, y);
    QGraphicsScene::addItem(item);

    // Advance
    if (++i % columns) x += toolWidth + margin;
    else {
      y += toolHeight + margin;
      x = margin;
    }
  }

  QGraphicsScene::update();
}


ToolGraphicsItem *ToolTableScene::toolAt(const QPointF &p) {
  return dynamic_cast<ToolGraphicsItem *>(itemAt(p, QTransform()));
}


void ToolTableScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  QGraphicsScene::contextMenuEvent(event);

  ToolGraphicsItem *item = toolAt(event->scenePos());
  if (item) {
    unsigned number = item->getNumber();

    QString suffix;
    suffix.sprintf(" tool #%d", number);

    QMenu menu;

    menu.addAction(QIcon(":/icons/add.png"), "Add tool");
    menu.addAction(QIcon(":/icons/edit.png"), QString("Edit") + suffix);
    menu.addAction(QIcon(":/icons/remove.png"), QString("Delete") + suffix);

    QAction *action = menu.exec(event->screenPos());
    if (!action) return;

    switch (action->text()[0].toLatin1()) {
    case 'A': emit addTool(); break;
    case 'E': emit editTool(number); break;
    case 'D': emit removeTool(number); break;
    }
  }
}


void ToolTableScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsScene::mouseDoubleClickEvent(event);

  ToolGraphicsItem *item = toolAt(event->scenePos());
  if (item) emit editTool(item->getNumber());
}
