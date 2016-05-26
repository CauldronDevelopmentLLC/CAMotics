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

#ifndef CAMOTICS_TOOL_DIALOG_H
#define CAMOTICS_TOOL_DIALOG_H

#include "ToolScene.h"

#include <cbang/SmartPointer.h>

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif

namespace Ui {class ToolDialog;}


namespace CAMotics {
  class ToolDialog : public QDialog {
    Q_OBJECT;

    cb::SmartPointer<Ui::ToolDialog> ui;
    ToolScene scene;
    Tool tool;

  public:
    ToolDialog();

    void setTool(const Tool &tool) {this->tool = tool;}
    Tool &getTool() {return tool;}

    int edit();
    void update();

    // From QDialog
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

  protected slots:
    void on_numberSpinBox_valueChanged(int value);
    void on_unitsComboBox_currentIndexChanged(int value);
    void on_shapeComboBox_currentIndexChanged(int value);
    void on_lengthDoubleSpinBox_valueChanged(double value);
    void on_diameterDoubleSpinBox_valueChanged(double value);
    void on_snubDiameterDoubleSpinBox_valueChanged(double value);
    void on_descriptionLineEdit_textChanged(const QString &value);

  };
}

#endif // CAMOTICS_TOOL_DIALOG_H
