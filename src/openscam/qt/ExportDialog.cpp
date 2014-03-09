/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

using namespace OpenSCAM;


ExportDialog::ExportDialog() : ui(new Ui::ExportDialog) {
  ui->setupUi(this);
}


void ExportDialog::enableSurface(bool enable) {
  ui->surfaceFrame->setEnabled(enable);
  ui->surfaceRadioButton->setEnabled(enable);
  if (!enable) ui->toolPathRadioButton->setChecked(true);
}


void ExportDialog::enableToolPath(bool enable) {
  ui->toolPathFrame->setEnabled(enable);
  ui->toolPathRadioButton->setEnabled(enable);
  if (!enable) ui->surfaceRadioButton->setChecked(true);
}


bool ExportDialog::surfaceSelected() const {
  return ui->surfaceRadioButton->isChecked();
}


bool ExportDialog::binarySTLSelected() const {
  return ui->binarySTLRadioButton->isChecked();
}


bool ExportDialog::compactJSONSelected() const {
  return ui->compactJSONRadioButton->isChecked();
}
