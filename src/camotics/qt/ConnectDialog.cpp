/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "ui_connect_dialog.h"

#include <QSettings>

using namespace CAMotics;


ConnectDialog::ConnectDialog() : ui(new Ui::ConnectDialog) {ui->setupUi(this);}


QString ConnectDialog::getAddress() const {return ui->addressLineEdit->text();}


void ConnectDialog::setFilename(const QString &filename) {
  ui->filenameLineEdit->setText(filename);

  ui->filenameLineEdit->setVisible(!filename.isEmpty());
  ui->filenameLabel->setVisible(!filename.isEmpty());

  if (filename.isEmpty())
    ui->plainTextEdit->setPlainText("Connect to a Buildbotics CNC controller?");
  else ui->plainTextEdit->setPlainText("Connect to a Buildbotics CNC "
                                       "controller and upload GCode?");
}


QString ConnectDialog::getFilename() const {
  return ui->filenameLineEdit->text();
}


int ConnectDialog::exec() {
  QSettings settings;

  QString addr = settings.value("Connect/Address", "bbctrl.local").toString();
  ui->addressLineEdit->setText(addr);

  return QDialog::exec();
}


void ConnectDialog::on_cancelPushButton_clicked() {reject();}


void ConnectDialog::on_connectPushButton_clicked() {accept();}
