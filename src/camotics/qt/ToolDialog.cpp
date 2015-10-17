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

#include "ToolDialog.h"

#include "ui_tool_dialog.h"

using namespace std;
using namespace CAMotics;


ToolDialog::ToolDialog() : ui(new Ui::ToolDialog) {
  ui->setupUi(this);
  ui->toolView->setScene(&scene);
}


int ToolDialog::edit() {
  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 1.0 / 25.4;

  ui->toolUnitsComboBox->setCurrentIndex(tool.getUnits());
  ui->shapeComboBox->setCurrentIndex(tool.getShape());
  ui->lengthDoubleSpinBox->setValue(tool.getLength() * scale);
  ui->diameterDoubleSpinBox->setValue(tool.getDiameter() * scale);
  ui->snubDiameterDoubleSpinBox->setValue(tool.getSnubDiameter() * scale);
  ui->descriptionLineEdit->
    setText(QString::fromUtf8(tool.getDescription().c_str()));

  on_shapeComboBox_currentIndexChanged(tool.getShape());

  update();

  return exec();
}


void ToolDialog::update() {
  scene.update(tool, ui->toolView->frameSize());
}


void ToolDialog::resizeEvent(QResizeEvent *event) {
  QDialog::resizeEvent(event);
  update();
}


void ToolDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  update();
}


void ToolDialog::on_toolUnitsComboBox_currentIndexChanged(int value) {
  ToolUnits units = (ToolUnits::enum_t)value;

  double step = units == ToolUnits::UNITS_MM ? 1 : 0.125;
  ui->lengthDoubleSpinBox->setSingleStep(step);
  ui->diameterDoubleSpinBox->setSingleStep(step);
  ui->snubDiameterDoubleSpinBox->setSingleStep(step);

  if (units == tool.getUnits()) return;

  tool.setUnits(units);
  update();
}


void ToolDialog::on_shapeComboBox_currentIndexChanged(int value) {
  ToolShape shape = (ToolShape::enum_t)value;

  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  double length = tool.getLength();
  double radius = tool.getRadius();
  if (shape == ToolShape::TS_BALLNOSE && length < radius)
    ui->lengthDoubleSpinBox->setValue(radius / scale);

  ui->snubDiameterDoubleSpinBox->setVisible(shape == ToolShape::TS_SNUBNOSE);
  ui->snubDiameterLabel->setVisible(shape == ToolShape::TS_SNUBNOSE);

  if (shape == tool.getShape()) return;

  tool.setShape(shape);
  update();
}


void ToolDialog::on_lengthDoubleSpinBox_valueChanged(double value) {
  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  double radius = tool.getRadius();
  if (tool.getShape() == ToolShape::TS_BALLNOSE && value < radius) {
    value = radius;
    ui->lengthDoubleSpinBox->setValue(value / scale);
  }

  if (value == tool.getLength()) return;

  tool.setLength(value);
  update();
}


void ToolDialog::on_diameterDoubleSpinBox_valueChanged(double value) {
  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  double length = tool.getLength();
  if (tool.getShape() == ToolShape::TS_BALLNOSE && length < value / 2)
    ui->lengthDoubleSpinBox->setValue(value / 2 / scale);

  if (value == tool.getDiameter()) return;

  tool.setDiameter(value);
  update();
}


void ToolDialog::on_snubDiameterDoubleSpinBox_valueChanged(double value) {
  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  value *= scale;

  if (value == tool.getSnubDiameter()) return;

  tool.setSnubDiameter(value);
  update();
}


void ToolDialog::on_descriptionLineEdit_textChanged(const QString &value) {
  string description = value.toUtf8().data();

  if (description == tool.getDescription()) return;

  tool.setDescription(description);
  update();
}
