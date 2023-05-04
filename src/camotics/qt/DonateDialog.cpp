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

#include "DonateDialog.h"

#include <cbang/Info.h>

#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>

using namespace CAMotics;
using namespace cb;


DonateDialog::DonateDialog(QWidget *parent) : Dialog(parent) {ui.setupUi(this);}


QString DonateDialog::getVersion() const {
  return QString::fromUtf8(Info::instance().get("CAMotics", "Version").c_str());
}


void DonateDialog::onStartup() {
  if (QSettings().value("Donate/ShowVersion", "") != getVersion()) exec();
}


int DonateDialog::exec() {
  int ret = QDialog::exec();
  QSettings().setValue("Donate/ShowVersion", getVersion());
  return ret;
}


void DonateDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);

  // Resize dialog to fit content up to screen height
  double h = ui.textBrowser->document()->size().height();
  h += height() - ui.textBrowser->size().height() + 5;

  double maxH = QApplication::desktop()->screenGeometry().height() - 5;
  if (maxH < h) h = maxH;

  setMaximumSize(width(), h);
  setMinimumSize(width(), h);
}
