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

#include "ConsoleWriter.h"
#include "QtWin.h"

#include <cbang/String.h>
#include <cbang/log/Logger.h>
#include <cbang/util/SmartLock.h>

#include <QColor>
#include <QByteArray>
#include <QTextBlock>

#include <iostream>

using namespace std;
using namespace cb;
using namespace CAMotics;


void ConsoleWriter::writeToConsole() {
  // Only access lines while holding cb::Logger lock
  SmartLock lock(&Logger::instance());

  for (unsigned i = 0; i < lines.size(); i++) {
    string &line = lines[i];

    cout << line << endl;

    QColor saveColor = QTextEdit::textColor();

    if (4 < line.size() && line[0] == 27 && line[1] == '[' && line[4] == 'm') {

      int code = String::parseU8(line.substr(2, 2));
      QColor color;

      switch (code) {
      case 30: color = QColor("#000000"); break;
      case 31: color = QColor("#ff0000"); break;
      case 32: color = QColor("#00ff00"); break;
      case 33: color = QColor("#ffff00"); break;
      case 34: color = QColor("#0000ff"); break;
      case 35: color = QColor("#ff00ff"); break;
      case 36: color = QColor("#00ffff"); break;
      case 37: color = QColor("#ffffff"); break;

      case 90: color = QColor("#555555"); break;
      case 91: color = QColor("#ff5555"); break;
      case 92: color = QColor("#55ff55"); break;
      case 93: color = QColor("#ffff55"); break;
      case 94: color = QColor("#5555ff"); break;
      case 95: color = QColor("#ff55ff"); break;
      case 96: color = QColor("#55ffff"); break;
      case 97: color = QColor("#ffffff"); break;
      }

      line = line.substr(5);
      QTextEdit::setTextColor(color);
    }

    if (String::endsWith(line, "\033[0m"))
      line = line.substr(0, line.size() - 4);

    QTextEdit::append(QByteArray(line.c_str()));
    QTextEdit::setTextColor(saveColor);
  }

  lines.clear();
}


void ConsoleWriter::mouseDoubleClickEvent(QMouseEvent *e) {
  QTextEdit::mouseDoubleClickEvent(e);

  QByteArray array = textCursor().block().text().toUtf8();
  string line = string(array.data(), array.length());

  vector<string> groups;
  String::find(line, "((ERROR)|(WARNING)):"
               "\\s*(at)?\\s*(([^:]+:[^\\d])?[^:]+):(\\d+)(:(\\d+))?", &groups);

  if (7 < groups.size()) {
    string filename = groups[5];
    uint32_t line = String::parseU32(groups[7]);
    uint32_t column = groups[9].empty() ? 0 : String::parseU32(groups[9]);

    QtWin *win = dynamic_cast<QtWin *>(window());
    if (!win) LOG_ERROR("Failed to get window");
    else win->activateFile(filename, line, column);
  }
}
