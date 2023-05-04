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

#include "UploadDialog.h"

#include <QSettings>

using namespace CAMotics;


UploadDialog::UploadDialog(QWidget *parent) : Dialog(parent) {ui.setupUi(this);}


QString UploadDialog::getFilename() const {
  return ui.filenameLineEdit->text();
}


void UploadDialog::setFilename(QString filename) {
  ui.filenameLineEdit->setText(filename);
}


bool UploadDialog::isAutomatic() const {
  return ui.automaticCheckBox->isChecked();
}


int UploadDialog::exec() {
  QSettings settings;

  bool automatic = settings.value("Upload/Automatic", true).toBool();
  ui.automaticCheckBox->setChecked(automatic);

  int ret = QDialog::exec();

  if (ret == QDialog::Accepted)
    settings.setValue("Upload/Automatic", isAutomatic());

  return ret;
}


void UploadDialog::on_uploadPushButton_clicked() {accept();}
