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

#pragma once

#include "Dialog.h"
#include "Settings.h"
#include "ui_settings_dialog.h"

#include <camotics/project/Project.h>
#include <camotics/view/View.h>

#include <gcode/plan/PlannerConfig.h>

#include <QDialog>


namespace GCode {class Axes;}

namespace CAMotics {
  class SettingsDialog : public Dialog {
    Q_OBJECT;
    CAMOTICS_DIALOG(SettingsDialog);

    Settings settings;
    cb::Rectangle3D bounds;
    bool changing = false;
    int selectedMachine = -1;

    GCode::PlannerConfig planConf;

  public:
    SettingsDialog(QWidget *parent);

    void setPlannerConfig(const GCode::PlannerConfig &c) {planConf = c;}
    const GCode::PlannerConfig &getPlannerConfig() const {return planConf;}

    void addMachine(const std::string &name, const std::string &path);
    std::string getMachineName() const;
    std::string getMachinePath() const;
    std::string getMachinePath(const std::string &machine) const;

    bool getPlannerEnabled() const;
    void setPlannerEnabled(bool enabled);

    void loadPlanVec(const std::string &widget, const std::string &var,
                     GCode::Axes &vec, double scale = 1);
    void savePlanVec(const std::string &widget, const std::string &var,
                     GCode::Axes &vec, double scale = 1);

    void loadPlanConfig();
    void savePlanConfig();

    void load(Project::Project &project, View &view);
    void save(Project::Project &project, View &view);

    bool exec(Project::Project &project, View &view);
    using QDialog::exec;

  signals:
    void machineChanged(QString machine, QString file);

  protected slots:
    void on_machineComboBox_currentIndexChanged(int index);
    void on_resolutionComboBox_currentIndexChanged(int index);
    void on_resolutionDoubleSpinBox_valueChanged(double value);
    void on_plannerEnableCheckBox_stateChanged(int checked);
  };
}
