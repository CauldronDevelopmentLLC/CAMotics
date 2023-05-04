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

#include "ConnectDialog.h"

#include <QSettings>

using namespace CAMotics;
using namespace std;


ConnectDialog::ConnectDialog(QWidget *parent) : Dialog(parent) {ui.setupUi(this);}


QString ConnectDialog::getAddress() const {return ui.addressLineEdit->text();}


bool ConnectDialog::isSystemProxyEnabled() const {
  return ui.systemProxyCheckBox->isChecked();
}


void ConnectDialog::setNetworkStatus(const string &status) {
  ui.networkStatusLabel->setText(status.c_str());

  if (status == "Disconnected") {
    ui.connectPushButton->setEnabled(true);
    ui.disconnectPushButton->setEnabled(false);

  } else {
    ui.connectPushButton->setEnabled(false);
    ui.disconnectPushButton->setEnabled(true);
  }

  string bg;
  if (status == "Connected")    bg = "#00ff00";
  else if (status == "Disconnected") bg = "#ffaa00";
  else bg = "#f2ad46";

  string css ="padding: 3px; background: " + bg;
  ui.networkStatusLabel->setStyleSheet(css.c_str());
}


int ConnectDialog::exec() {
  QSettings settings;

  QString addr = settings.value("Connect/Address", "bbctrl").toString();
  ui.addressLineEdit->setText(addr);

  bool useProxy = settings.value("Connect/UseSystemProxy", true).toBool();
  ui.systemProxyCheckBox->setChecked(useProxy);

  int ret = QDialog::exec();

  if (ret == QDialog::Accepted) {
    settings.setValue("Connect/Address", ui.addressLineEdit->text());
    settings.setValue("Connect/UseSystemProxy", isSystemProxyEnabled());
  }

  return ret;
}


void ConnectDialog::on_disconnectPushButton_clicked() {
  emit disconnect();
  accept();
}


void ConnectDialog::on_connectPushButton_clicked() {
  setNetworkStatus("Connecting");
  emit connect();
}
