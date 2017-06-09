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

#include <gcode/ToolUnits.h>
#include <camotics/cam/CAMLayer.h>

#include <cbang/SmartPointer.h>

#include <QDialog>
#include <QTableWidget>


namespace Ui {class CAMLayerDialog;}


namespace CAMotics {
  class CAMLayerDialog : public QDialog {
    Q_OBJECT;

    cb::SmartPointer<Ui::CAMLayerDialog> ui;

  public:
    CAMLayerDialog(QWidget *parent);

    void setLayers(const std::vector<std::string> &layers);
    void setUnits(GCode::ToolUnits units);

    CAMLayer getLayer() const;
    void setLayer(const CAMLayer &layer);

    void update();
    int exec();

  protected slots:
    void on_offsetComboBox_currentIndexChanged(int) {update();}
    void on_startDepthDoubleSpinBox_valueChanged(double x);
    void on_endDepthDoubleSpinBox_valueChanged(double x);
  };
}
