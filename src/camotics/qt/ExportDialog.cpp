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

#include "ExportDialog.h"

#include "ui_export_dialog.h"

using namespace CAMotics;


ExportDialog::ExportDialog() : ui(new Ui::ExportDialog) {
  ui->setupUi(this);
}


void ExportDialog::enableSurface(bool enable) {
  ui->surfaceFrame->setEnabled(enable && surfaceSelected());
  ui->surfaceRadioButton->setEnabled(enable);
}


void ExportDialog::enableGCode(bool enable) {
  ui->gcodeFrame->setEnabled(enable && gcodeSelected());
  ui->gcodeRadioButton->setEnabled(enable);
}


void ExportDialog::enableSimData(bool enable) {
  ui->simDataFrame->setEnabled(enable && simDataSelected());
  ui->simDataRadioButton->setEnabled(enable);
}


bool ExportDialog::surfaceSelected() const {
  return ui->surfaceRadioButton->isChecked();
}


bool ExportDialog::gcodeSelected() const {
  return ui->gcodeRadioButton->isChecked();
}


bool ExportDialog::simDataSelected() const {
  return ui->simDataRadioButton->isChecked();
}


bool ExportDialog::binarySTLSelected() const {
  return ui->binarySTLRadioButton->isChecked();
}


bool ExportDialog::metricUnitsSelected() const {
  return ui->metricUnitsRadioButton->isChecked();
}


bool ExportDialog::compactJSONSelected() const {
  return ui->compactJSONRadioButton->isChecked();
}


void ExportDialog::on_surfaceRadioButton_clicked() {
  ui->surfaceFrame->setEnabled(true);
  ui->gcodeFrame->setEnabled(false);
  ui->simDataFrame->setEnabled(false);
}


void ExportDialog::on_gcodeRadioButton_clicked() {
  ui->surfaceFrame->setEnabled(false);
  ui->gcodeFrame->setEnabled(true);
  ui->simDataFrame->setEnabled(false);
}


void ExportDialog::on_simDataRadioButton_clicked() {
  ui->surfaceFrame->setEnabled(false);
  ui->gcodeFrame->setEnabled(false);
  ui->simDataFrame->setEnabled(true);
}
