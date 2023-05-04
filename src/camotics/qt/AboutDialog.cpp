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

#include "AboutDialog.h"

#include <cbang/Info.h>
#include <cbang/log/Logger.h>

#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>

using namespace std;
using namespace cb;
using namespace CAMotics;


AboutDialog::AboutDialog(QWidget *parent) : Dialog(parent) {
  ui.setupUi(this);

  // Set version
  string cat = "CAMotics";
  string version   = Info::instance().get(cat, "Version");
  string copyright = Info::instance().get(cat, "Copyright");
  string author    = Info::instance().get(cat, "Author");
  string license   = Info::instance().get(cat, "License");

  QString html = ui.textBrowser->toHtml();
  html.replace("$VERSION",   QString::fromUtf8(version.c_str()));
  html.replace("$COPYRIGHT", QString::fromUtf8(copyright.c_str()));
  html.replace("$AUTHOR",    QString::fromUtf8(author.c_str()));
  html.replace("$LICENSE",   QString::fromUtf8(license.c_str()));
  ui.textBrowser->setHtml(html);
}


void AboutDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);

  // Resize dialog to fit content up to screen height
  double h = ui.textBrowser->document()->size().height();
  h += height() - ui.textBrowser->size().height() + 5;

  double maxH = QApplication::desktop()->screenGeometry().height() - 5;
  if (maxH < h) h = maxH;

  setMaximumSize(width(), h);
  setMinimumSize(width(), h);
}
