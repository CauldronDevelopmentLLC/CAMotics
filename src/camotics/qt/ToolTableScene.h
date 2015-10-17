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

#ifndef CAMOTICS_TOOL_TABLE_SCENE_H
#define CAMOTICS_TOOL_TABLE_SCENE_H

#include <camotics/sim/ToolTable.h>

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif


namespace CAMotics {
  class ToolGraphicsItem;

  class ToolTableScene : public QGraphicsScene {
    Q_OBJECT;

  public:
    void update(const ToolTable &tools, QSize dims);

    ToolGraphicsItem *toolAt(const QPointF &p);

    // From QGraphicsScene
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

  signals:
    void addTool();
    void editTool(unsigned number);
    void removeTool(unsigned number);
  };
}

#endif // CAMOTICS_TOOL_TABLE_SCENE_H

