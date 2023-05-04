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
#include "ToolScene.h"
#include "ui_tool_dialog.h"

#include <cbang/SmartPointer.h>

#include <QtWidgets>


namespace CAMotics {
  class ToolDialog : public Dialog, public GCode::ToolShape {
    Q_OBJECT;
    CAMOTICS_DIALOG(ToolDialog);
    
    ToolScene scene;
    GCode::Tool tool;
    bool updating;

  public:
    ToolDialog(QWidget *parent);

    void setTool(const GCode::Tool &tool) {this->tool = tool;}
    GCode::Tool &getTool() {return tool;}

    bool isMetric() const;
    double getScale() const;

    int edit();

    void updateNumber();
    void updateUnits();
    void updateShape();
    void updateAngle();
    void limitLength();
    void updateLength();
    void updateDiameter();
    void updateSnubDiameter();
    void updateDescription();
    void updateScene();
    void update();

    // From QDialog
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

  protected slots:
    void on_numberSpinBox_valueChanged(int value);
    void on_unitsComboBox_currentIndexChanged(int value);
    void on_shapeComboBox_currentIndexChanged(int value);
    void on_angleDoubleSpinBox_valueChanged(double value);
    void on_lengthDoubleSpinBox_valueChanged(double value);
    void on_diameterDoubleSpinBox_valueChanged(double value);
    void on_snubDiameterDoubleSpinBox_valueChanged(double value);
    void on_descriptionLineEdit_textChanged(const QString &value);
  };
}
