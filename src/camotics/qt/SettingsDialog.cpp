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

#include "SettingsDialog.h"

#include "ui_settings_dialog.h"

#include <QSettings>

using namespace std;
using namespace cb;
using namespace CAMotics;


SettingsDialog::SettingsDialog() : ui(new Ui::SettingsDialog), changing(false) {
  ui->setupUi(this);

#ifndef DEBUG
  // Hide render mode controls
  ui->renderModeLabel->setVisible(false);
  ui->renderModeComboBox->setVisible(false);
#endif
}


void SettingsDialog::exec(Project &project) {
  bounds = project.getWorkpieceBounds();

  ui->resolutionDoubleSpinBox->setValue(project.getResolution());
  ui->resolutionComboBox->setCurrentIndex(project.getResolutionMode());
  ui->renderModeComboBox->setCurrentIndex(project.getRenderMode() - 1);
  ui->unitsComboBox->setCurrentIndex(project.getUnits());

  QSettings settings;

  ui->defaultUnitsComboBox->
    setCurrentIndex(settings.value("Settings/Units").toInt());

  if (QDialog::exec() != QDialog::Accepted) return;

  project.setResolution(ui->resolutionDoubleSpinBox->value());

  int index = ui->resolutionComboBox->currentIndex();
  project.setResolutionMode((ResolutionMode::enum_t)index);

  index = ui->renderModeComboBox->currentIndex() + 1;
  project.setRenderMode((RenderMode::enum_t)index);

  ToolUnits units = (ToolUnits::enum_t)ui->unitsComboBox->currentIndex();
  project.setUnits(units);

  settings.setValue("Settings/Units", ui->defaultUnitsComboBox->currentIndex());
}


void SettingsDialog::on_resolutionComboBox_currentIndexChanged(int index) {
  if (changing) return;

  ResolutionMode mode = (ResolutionMode::enum_t)index;
  double resolution = Project::computeResolution(mode, bounds);

  changing = true;
  ui->resolutionDoubleSpinBox->setValue(resolution);
  changing = false;
}


void SettingsDialog::on_resolutionDoubleSpinBox_valueChanged(double value) {
  if (changing) return;

  changing = true;
  ui->resolutionComboBox->setCurrentIndex(ResolutionMode::RESOLUTION_MANUAL);
  changing = false;
}
