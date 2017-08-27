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

#pragma once


#include <camotics/sim/Project.h>
#include <camotics/view/View.h>

#include <cbang/SmartPointer.h>

#include <QDialog>

namespace Ui {class SettingsDialog;}


namespace CAMotics {
  class SettingsDialog : public QDialog {
    Q_OBJECT;

    cb::SmartPointer<Ui::SettingsDialog> ui;

    cb::Rectangle3D bounds;
    bool changing;

  public:
    SettingsDialog(QWidget *parent);

    void addMachine(const std::string &name, const std::string &path);
    std::string getMachineName() const;
    std::string getMachinePath() const;
    std::string getMachinePath(const std::string &machine) const;

    bool exec(Project &project, View &view);
    using QDialog::exec;

  signals:
    void machineChanged(QString machine, QString file);

  protected slots:
    void on_machineComboBox_currentIndexChanged(int index);
    void on_resolutionComboBox_currentIndexChanged(int index);
    void on_resolutionDoubleSpinBox_valueChanged(double value);
  };
}
