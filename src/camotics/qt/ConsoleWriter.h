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


#include <QTextEdit>
#include <QMenu>

#include <vector>


namespace CAMotics {
  class ConsoleWriter : public QTextEdit {
    Q_OBJECT;

    QMenu menu;

    std::vector<std::string> lines;

  public:
    ConsoleWriter(QWidget *parent = 0);

    void append(const std::string &line) {lines.push_back(line);}
    void writeToConsole();

    // From QTextEdit
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);

    bool findOnce(QString text, bool regex, int options);

  signals:
    void find();
    void findNext();
    void findResult(bool);

  protected slots:
    void on_find(QString text, bool regex, int options);
  };
}
