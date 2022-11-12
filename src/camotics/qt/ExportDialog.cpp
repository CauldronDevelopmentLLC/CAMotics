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

#include "ui_export_dialog.h"

#include <QSettings>

using namespace CAMotics;


#define UI() Dialog::getUI<Ui::ExportDialog>()


ExportDialog::ExportDialog(QWidget *parent) :
  Dialog(parent, new UI<Ui::ExportDialog>) {
#ifdef _WIN32
  bool crlf = true;
#else
  bool crlf = false;
#endif

  crlf = QSettings().value("Settings/GCode/CRLF", crlf).toBool();
  UI().crlfCheckBox->setChecked(crlf);
}


int ExportDialog::exec() {
  bool change = false;

  if (!UI().gcodeRadioButton->isEnabled() && gcodeSelected()) change = true;
  if (!UI().surfaceRadioButton->isEnabled() && surfaceSelected()) change = true;
  if (!UI().simDataRadioButton->isEnabled() && simDataSelected()) change = true;

  if (change) {
    if (UI().gcodeRadioButton->isEnabled()) {
      UI().gcodeRadioButton->setChecked(true);
      on_gcodeRadioButton_clicked();

    } else if (UI().surfaceRadioButton->isEnabled()) {
      UI().surfaceRadioButton->setChecked(true);
      on_surfaceRadioButton_clicked();

    } else if (UI().simDataRadioButton->isEnabled()) {
      UI().simDataRadioButton->setChecked(true);
      on_simDataRadioButton_clicked();
    }
  }

  int ret = QDialog::exec();

  if (ret == QDialog::Accepted)
    QSettings().setValue("Settings/GCode/CRLF", crlfSelected());

  return ret;
}


void ExportDialog::enableSurface(bool enable) {
  UI().surfaceFrame->setEnabled(enable && surfaceSelected());
  UI().surfaceRadioButton->setEnabled(enable);
  UI().cutSurfaceCheckBox->setEnabled(enable);
  if (!enable) UI().cutSurfaceCheckBox->setChecked(false);
}


void ExportDialog::enableGCode(bool enable) {
  UI().gcodeRadioButton->setEnabled(enable);
}


void ExportDialog::enableSimData(bool enable) {
  UI().simDataFrame->setEnabled(enable && simDataSelected());
  UI().simDataRadioButton->setEnabled(enable);
}


bool ExportDialog::crlfSelected() const {
  return UI().crlfCheckBox->isChecked();
}


bool ExportDialog::surfaceSelected() const {
  return UI().surfaceRadioButton->isChecked();
}


bool ExportDialog::gcodeSelected() const {
  return UI().gcodeRadioButton->isChecked();
}


bool ExportDialog::simDataSelected() const {
  return UI().simDataRadioButton->isChecked();
}


bool ExportDialog::binarySTLSelected() const {
  return UI().binarySTLRadioButton->isChecked();
}


bool ExportDialog::compactJSONSelected() const {
  return UI().compactJSONRadioButton->isChecked();
}


bool ExportDialog::withCutSurfaceSelected() const {
  return UI().cutSurfaceCheckBox->isChecked();
}


void ExportDialog::on_surfaceRadioButton_clicked() {
  UI().surfaceFrame->setEnabled(true);
  UI().simDataFrame->setEnabled(false);
}


void ExportDialog::on_gcodeRadioButton_clicked() {
  UI().surfaceFrame->setEnabled(false);
  UI().simDataFrame->setEnabled(false);
}


void ExportDialog::on_simDataRadioButton_clicked() {
  UI().surfaceFrame->setEnabled(false);
  UI().simDataFrame->setEnabled(true);
}
