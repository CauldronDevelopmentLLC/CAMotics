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

#include "SettingsDialog.h"
#include "Settings.h"

#include "ui_settings_dialog.h"

#include <camotics/view/GL.h>

using namespace std;
using namespace cb;
using namespace CAMotics;


SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::SettingsDialog), changing(false) {
  ui->setupUi(this);

#ifndef DEBUG
  // Hide advanced controls
  ui->tabWidget->removeTab(1);
#endif

  ui->tabWidget->setCurrentIndex(0); // Select first tab
}


void SettingsDialog::addMachine(const string &name, const string &path) {
  ui->machineComboBox->addItem(QString::fromUtf8(name.c_str()),
                               QString::fromUtf8(path.c_str()));
}


string SettingsDialog::getMachineName() const {
  return ui->machineComboBox->currentText().toUtf8().data();
}


string SettingsDialog::getMachinePath(const string &machine) const {
  int i = ui->machineComboBox->findText(QString::fromUtf8(machine.c_str()));
  if (i == -1) THROWS("Machine '" << machine << "' not found");
  return ui->machineComboBox->itemData(i).toString().toUtf8().data();
}


string SettingsDialog::getMachinePath() const {
  return ui->machineComboBox->currentData().toString().toUtf8().data();
}


bool SettingsDialog::exec(Project::Project &project, View &view) {
  Settings settings;

  bounds = project.getWorkpiece().getBounds();

  // Select machine
  int selectedMachine = ui->machineComboBox->findText
    (settings.get("Settings/Machine", "Taig Mini Mill").toString());
  if (selectedMachine != -1)
    ui->machineComboBox->setCurrentIndex(selectedMachine);

  ui->resolutionDoubleSpinBox->setValue(project.getResolution());
  ui->resolutionComboBox->setCurrentIndex(project.getResolutionMode());
  ui->unitsComboBox->setCurrentIndex(project.getUnits());

  ui->defaultUnitsComboBox->
    setCurrentIndex(settings.get("Settings/Units",
                                 GCode::Units::METRIC).toInt());

  ui->renderModeComboBox->
    setCurrentIndex(settings.get("Settings/RenderMode", 0).toInt());

  ui->aabbCheckBox->setChecked(view.isFlagSet(View::SHOW_BBTREE_FLAG));
  ui->aabbLeavesCheckBox->setChecked(view.isFlagSet(View::BBTREE_LEAVES_FLAG));

  if (QDialog::exec() != QDialog::Accepted) {
    if (selectedMachine != -1)
      ui->machineComboBox->setCurrentIndex(selectedMachine);
    return false;
  }

  settings.set("Settings/Machine", ui->machineComboBox->currentText());

  ResolutionMode mode =
    (ResolutionMode::enum_t)ui->resolutionComboBox->currentIndex();
  project.setResolutionMode(mode);

  if (mode == ResolutionMode::RESOLUTION_MANUAL)
    project.setResolution(ui->resolutionDoubleSpinBox->value());

  GCode::Units units =
    (GCode::Units::enum_t)ui->unitsComboBox->currentIndex();
  project.setUnits(units);
  settings.set("Settings/Units", ui->defaultUnitsComboBox->currentIndex());

  settings.set("Settings/RenderMode", ui->renderModeComboBox->currentIndex());

  view.setFlag(View::SHOW_BBTREE_FLAG, ui->aabbCheckBox->isChecked());
  view.setFlag(View::BBTREE_LEAVES_FLAG, ui->aabbLeavesCheckBox->isChecked());

  return true;
}


void SettingsDialog::on_machineComboBox_currentIndexChanged(int index) {
  emit machineChanged(ui->machineComboBox->currentText(),
                      ui->machineComboBox->currentData().toString());
}


void SettingsDialog::on_resolutionComboBox_currentIndexChanged(int index) {
  if (changing) return;

  ResolutionMode mode = (ResolutionMode::enum_t)index;
  double resolution = Project::Project::computeResolution(mode, bounds);

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
