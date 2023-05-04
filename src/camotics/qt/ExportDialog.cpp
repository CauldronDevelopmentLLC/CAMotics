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

#include "ExportDialog.h"

#include <QSettings>

using namespace CAMotics;


ExportDialog::ExportDialog(QWidget *parent) : Dialog(parent) {
  ui.setupUi(this);

#ifdef _WIN32
  bool crlf = true;
#else
  bool crlf = false;
#endif

  crlf = QSettings().value("Settings/GCode/CRLF", crlf).toBool();
  ui.crlfCheckBox->setChecked(crlf);
}


int ExportDialog::exec() {
  bool change = false;

  if (!ui.gcodeRadioButton->isEnabled() && gcodeSelected()) change = true;
  if (!ui.surfaceRadioButton->isEnabled() && surfaceSelected()) change = true;
  if (!ui.simDataRadioButton->isEnabled() && simDataSelected()) change = true;

  if (change) {
    if (ui.gcodeRadioButton->isEnabled()) {
      ui.gcodeRadioButton->setChecked(true);
      on_gcodeRadioButton_clicked();

    } else if (ui.surfaceRadioButton->isEnabled()) {
      ui.surfaceRadioButton->setChecked(true);
      on_surfaceRadioButton_clicked();

    } else if (ui.simDataRadioButton->isEnabled()) {
      ui.simDataRadioButton->setChecked(true);
      on_simDataRadioButton_clicked();
    }
  }

  int ret = QDialog::exec();

  if (ret == QDialog::Accepted)
    QSettings().setValue("Settings/GCode/CRLF", crlfSelected());

  return ret;
}


void ExportDialog::enableSurface(bool enable) {
  ui.surfaceFrame->setEnabled(enable && surfaceSelected());
  ui.surfaceRadioButton->setEnabled(enable);
  ui.cutSurfaceCheckBox->setEnabled(enable);
  if (!enable) ui.cutSurfaceCheckBox->setChecked(false);
}


void ExportDialog::enableGCode(bool enable) {
  ui.gcodeRadioButton->setEnabled(enable);
}


void ExportDialog::enableSimData(bool enable) {
  ui.simDataFrame->setEnabled(enable && simDataSelected());
  ui.simDataRadioButton->setEnabled(enable);
}


bool ExportDialog::crlfSelected() const {
  return ui.crlfCheckBox->isChecked();
}


bool ExportDialog::surfaceSelected() const {
  return ui.surfaceRadioButton->isChecked();
}


bool ExportDialog::gcodeSelected() const {
  return ui.gcodeRadioButton->isChecked();
}


bool ExportDialog::simDataSelected() const {
  return ui.simDataRadioButton->isChecked();
}


bool ExportDialog::binarySTLSelected() const {
  return ui.binarySTLRadioButton->isChecked();
}


bool ExportDialog::compactJSONSelected() const {
  return ui.compactJSONRadioButton->isChecked();
}


bool ExportDialog::withCutSurfaceSelected() const {
  return ui.cutSurfaceCheckBox->isChecked();
}


void ExportDialog::on_surfaceRadioButton_clicked() {
  ui.surfaceFrame->setEnabled(true);
  ui.simDataFrame->setEnabled(false);
}


void ExportDialog::on_gcodeRadioButton_clicked() {
  ui.surfaceFrame->setEnabled(false);
  ui.simDataFrame->setEnabled(false);
}


void ExportDialog::on_simDataRadioButton_clicked() {
  ui.surfaceFrame->setEnabled(false);
  ui.simDataFrame->setEnabled(true);
}
