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

using namespace std;
using namespace cb;
using namespace CAMotics;


SettingsDialog::SettingsDialog(QWidget *parent) : Dialog(parent) {
  ui.setupUi(this);
  
#ifndef DEBUG
  // Hide advanced controls
  ui.tabWidget->removeTab(1);
#endif

  ui.tabWidget->setCurrentIndex(0); // Select first tab

  loadPlanConfig();
}


void SettingsDialog::addMachine(const string &name, const string &path) {
  if (ui.machineComboBox->findText(QString::fromUtf8(name.c_str())) < 0)
    ui.machineComboBox->addItem(QString::fromUtf8(name.c_str()),
                                 QString::fromUtf8(path.c_str()));
}


string SettingsDialog::getMachineName() const {
  return ui.machineComboBox->currentText().toUtf8().data();
}


string SettingsDialog::getMachinePath(const string &machine) const {
  int i = ui.machineComboBox->findText(QString::fromUtf8(machine.c_str()));
  if (i == -1) THROW("Machine '" << machine << "' not found");
  return ui.machineComboBox->itemData(i).toString().toUtf8().data();
}


string SettingsDialog::getMachinePath() const {
  return ui.machineComboBox->currentData().toString().toUtf8().data();
}


bool SettingsDialog::getPlannerEnabled() const {
  return settings.get("Settings/Planner", 0).toInt();
}


void SettingsDialog::setPlannerEnabled(bool enabled) {
  settings.set("Settings/Planner", (int)enabled);
}


void SettingsDialog::loadPlanVec(const string &widget, const string &var,
                                 GCode::Axes &axes, double scale) {
  auto v = settings.getVector3D("Settings/" + var, axes.getXYZ() / scale);
  axes.setXYZ(v * scale);
  setVector3D(widget, v);
}


void SettingsDialog::savePlanVec(const string &widget, const string &var,
                                 GCode::Axes &vec, double scale) {
  auto v = getVector3D(widget);
  settings.setVector3D("Settings/" + var, v);
  vec.setXYZ(v * scale);
}


void SettingsDialog::loadPlanConfig() {
  bool enabled = getPlannerEnabled();
  ui.plannerGroupBox->setEnabled(enabled);
  ui.plannerEnableCheckBox->setChecked(enabled);

  double maxDeviation = planConf.maxBlendError;
  maxDeviation = settings.get("Settings/MaxDeviation", maxDeviation).toDouble();
  ui.maxDeviationDoubleSpinBox->setValue(maxDeviation);

  if (settings.has("Settings/MaxDeviation")) {
    planConf.maxBlendError = maxDeviation;
    planConf.maxMergeError = maxDeviation;
    planConf.maxArcError   = maxDeviation / 10;
  }

  loadPlanVec("maxVel",   "MaxVelocity",      planConf.maxVel,   1e3);
  loadPlanVec("maxAccel", "MaxAccelleration", planConf.maxAccel, 1e6);
  loadPlanVec("maxJerk",  "MaxJerk",          planConf.maxJerk,  1e6);
  loadPlanVec("min",      "MinCoordinates",   planConf.minSoftLimit);
  loadPlanVec("max",      "MaxCoordinates",   planConf.maxSoftLimit);
}


void SettingsDialog::savePlanConfig() {
  setPlannerEnabled(ui.plannerEnableCheckBox->isChecked());

  double maxDeviation = ui.maxDeviationDoubleSpinBox->value();
  settings.set("Settings/MaxDeviation", maxDeviation);
  planConf.maxBlendError = maxDeviation;
  planConf.maxMergeError = maxDeviation;
  planConf.maxArcError   = maxDeviation / 10;

  savePlanVec("maxVel",   "MaxVelocity",      planConf.maxVel,   1e3);
  savePlanVec("maxAccel", "MaxAccelleration", planConf.maxAccel, 1e6);
  savePlanVec("maxJerk",  "MaxJerk",          planConf.maxJerk,  1e6);
  savePlanVec("min",      "MinCoordinates",   planConf.minSoftLimit);
  savePlanVec("max",      "MaxCoordinates",   planConf.maxSoftLimit);
}


void SettingsDialog::load(Project::Project &project, View &view) {
  bounds = project.getWorkpiece().getBounds();

  // Select machine
  selectedMachine = ui.machineComboBox->findText
    (settings.get("Settings/Machine", "Taig Mini Mill").toString());
  if (selectedMachine != -1)
    ui.machineComboBox->setCurrentIndex(selectedMachine);

  ui.resolutionDoubleSpinBox->setValue(project.getResolution());
  ui.resolutionComboBox->setCurrentIndex(project.getResolutionMode());
  ui.unitsComboBox->setCurrentIndex(project.getUnits());

  ui.defaultUnitsComboBox->
    setCurrentIndex(settings.get("Settings/Units",
                                 GCode::Units::METRIC).toInt());

  ui.renderModeComboBox->
    setCurrentIndex(settings.get("Settings/RenderMode", 0).toInt());

  ui.aabbCheckBox->setChecked(view.isFlagSet(View::SHOW_BBTREE_FLAG));
  ui.aabbLeavesCheckBox->setChecked(view.isFlagSet(View::BBTREE_LEAVES_FLAG));

  loadPlanConfig();
}


void SettingsDialog::save(Project::Project &project, View &view) {
  settings.set("Settings/Machine", ui.machineComboBox->currentText());

  ResolutionMode mode =
    (ResolutionMode::enum_t)ui.resolutionComboBox->currentIndex();
  project.setResolutionMode(mode);

  if (mode == ResolutionMode::RESOLUTION_MANUAL)
    project.setResolution(ui.resolutionDoubleSpinBox->value());

  GCode::Units units =
    (GCode::Units::enum_t)ui.unitsComboBox->currentIndex();
  project.setUnits(units);
  settings.set("Settings/Units", ui.defaultUnitsComboBox->currentIndex());

  settings.set("Settings/RenderMode", ui.renderModeComboBox->currentIndex());

  view.setFlag(View::SHOW_BBTREE_FLAG, ui.aabbCheckBox->isChecked());
  view.setFlag(View::BBTREE_LEAVES_FLAG, ui.aabbLeavesCheckBox->isChecked());

  savePlanConfig();
}


bool SettingsDialog::exec(Project::Project &project, View &view) {
  load(project, view);

  if (QDialog::exec() != QDialog::Accepted) {
    if (selectedMachine != -1)
      ui.machineComboBox->setCurrentIndex(selectedMachine);
    return false;
  }

  save(project, view);

  return true;
}


void SettingsDialog::on_machineComboBox_currentIndexChanged(int index) {
  emit machineChanged(ui.machineComboBox->currentText(),
                      ui.machineComboBox->currentData().toString());
}


void SettingsDialog::on_resolutionComboBox_currentIndexChanged(int index) {
  if (changing) return;

  ResolutionMode mode = (ResolutionMode::enum_t)index;
  double resolution = Project::Project::computeResolution(mode, bounds);

  changing = true;
  ui.resolutionDoubleSpinBox->setValue(resolution);
  changing = false;
}


void SettingsDialog::on_resolutionDoubleSpinBox_valueChanged(double value) {
  if (changing) return;

  changing = true;
  ui.resolutionComboBox->setCurrentIndex(ResolutionMode::RESOLUTION_MANUAL);
  changing = false;
}


void SettingsDialog::on_plannerEnableCheckBox_stateChanged(int checked) {
  ui.plannerGroupBox->setEnabled(checked);
}
