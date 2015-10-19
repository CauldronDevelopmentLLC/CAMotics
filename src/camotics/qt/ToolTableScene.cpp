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


ToolTableScene::ToolTableScene() {}


void ToolTableScene::update(const ToolTable &tools, QSize dims) {
  QGraphicsScene::clear();

  if (tools.empty()) return;

  unsigned dpi = QDesktopWidget().logicalDpiX();
  if (dpi < 50 || 10000 < dpi) dpi = 95; // Correct for implausible DPI
  const unsigned toolWidth = dpi * 1.5;
  const unsigned toolHeight = toolWidth;
  const unsigned margin = 4;

  const unsigned columns =
    (unsigned)floor((dims.width() - margin) / (double)(toolWidth + margin));
  const unsigned rows = (unsigned)ceil(tools.size() / (double)columns);

  if (!columns) return;

  const unsigned width = dims.width();
  const unsigned height = (toolHeight + margin) * rows + margin;
  QGraphicsScene::setSceneRect(0, 0, width, height);

  unsigned i = 0;
  unsigned x = margin;
  unsigned y = margin;

  for (ToolTable::const_iterator it = tools.begin(); it != tools.end(); it++) {
    const Tool &tool = it->second;

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

  QMenu menu;
  unsigned number = 0;

  menu.addAction(QIcon(":/icons/add.png"), "Add tool");

  ToolGraphicsItem *item = toolAt(event->scenePos());
  if (item) {
    number = item->getNumber();

    QString suffix;
    suffix.sprintf(" tool #%d", number);

    menu.addAction(QIcon(":/icons/edit.png"), QString("Edit") + suffix);
    menu.addAction(QIcon(":/icons/remove.png"), QString("Delete") + suffix);
  }

  menu.addSeparator();
  menu.addAction(QIcon(":/icons/tool-import.png"), "Import tool table");
  menu.addAction(QIcon(":/icons/tool-export.png"), "Export tool table");

  QAction *action = menu.exec(event->screenPos());
  if (!action) return;

  QByteArray text = action->text().toLatin1();
  switch (text[0]) {
  case 'A': emit addTool(); break;
  case 'D': emit removeTool(number); break;
  case 'E':
    if (text[1] == 'd') emit editTool(number);
    else emit exportToolTable();
    break;
  case 'I': emit importToolTable(); break;
  }
}


void ToolTableScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsScene::mouseDoubleClickEvent(event);

  ToolGraphicsItem *item = toolAt(event->scenePos());
  if (item) emit editTool(item->getNumber());
}


void ToolTableScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
  if (event->mimeData()->hasFormat("application/x-camotics-tool")) {
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }

  QGraphicsScene::dragEnterEvent(event);
}


void ToolTableScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
  // Note, default implementation ignores drags if an item does not accept it
}


void ToolTableScene::dropEvent(QGraphicsSceneDragDropEvent *event) {
  QGraphicsScene::dropEvent(event);
}
