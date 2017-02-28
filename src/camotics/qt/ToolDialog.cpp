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

#include "ToolDialog.h"

#include "ui_tool_dialog.h"

using namespace std;
using namespace CAMotics;


ToolDialog::ToolDialog() : ui(new Ui::ToolDialog), updating(false) {
  ui->setupUi(this);
  ui->toolView->setScene(&scene);
}


int ToolDialog::edit() {
  update();
  return exec();
}


void ToolDialog::update() {
  updating = true;

  ToolUnits units = tool.getUnits();
  double scale = units == ToolUnits::UNITS_MM ? 1.0 : 1.0 / 25.4;
  ToolShape shape = tool.getShape();
  double length = tool.getLength();
  double radius = tool.getRadius();

  // Limits
  if (shape == ToolShape::TS_BALLNOSE && length < radius)
    tool.setLength(radius);

  // Step
  double step = units == ToolUnits::UNITS_MM ? 1 : 0.125;
  ui->lengthDoubleSpinBox->setSingleStep(step);
  ui->diameterDoubleSpinBox->setSingleStep(step);
  ui->snubDiameterDoubleSpinBox->setSingleStep(step);

  // Suffix
  const char *suffix = units == ToolUnits::UNITS_MM ? "mm" : "in";
  ui->lengthDoubleSpinBox->setSuffix(suffix);
  ui->diameterDoubleSpinBox->setSuffix(suffix);
  ui->snubDiameterDoubleSpinBox->setSuffix(suffix);

  // Visibility
  ui->angleDoubleSpinBox->setVisible(shape == ToolShape::TS_CONICAL);
  ui->angleLabel->setVisible(shape == ToolShape::TS_CONICAL);
  ui->snubDiameterDoubleSpinBox->setVisible(shape == ToolShape::TS_SNUBNOSE);
  ui->snubDiameterLabel->setVisible(shape == ToolShape::TS_SNUBNOSE);

  // Values
  ui->numberSpinBox->setValue(tool.getNumber());
  ui->unitsComboBox->setCurrentIndex(tool.getUnits());
  ui->shapeComboBox->setCurrentIndex(tool.getShape());
  ui->angleDoubleSpinBox->setValue(tool.getAngle());
  ui->lengthDoubleSpinBox->setValue(tool.getLength() * scale);
  ui->diameterDoubleSpinBox->setValue(tool.getDiameter() * scale);
  ui->snubDiameterDoubleSpinBox->setValue(tool.getSnubDiameter() * scale);
  ui->descriptionLineEdit->
    setText(QString::fromUtf8(tool.getDescription().c_str()));

  scene.update(tool, ui->toolView->frameSize());
  updating = false;
}


void ToolDialog::resizeEvent(QResizeEvent *event) {
  QDialog::resizeEvent(event);
  update();
}


void ToolDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  update();
}


void ToolDialog::on_numberSpinBox_valueChanged(int value) {
  if (updating) return;
  tool.setNumber((unsigned)value);
  update();
}


void ToolDialog::on_unitsComboBox_currentIndexChanged(int value) {
  if (updating) return;
  tool.setUnits((ToolUnits::enum_t)value);
  update();
}


void ToolDialog::on_shapeComboBox_currentIndexChanged(int value) {
  if (updating) return;
  tool.setShape((ToolShape::enum_t)value);
  update();
}


void ToolDialog::on_angleDoubleSpinBox_valueChanged(double angle) {
  if (updating) return;
  tool.setLengthFromAngle(angle);
  update();
}


void ToolDialog::on_lengthDoubleSpinBox_valueChanged(double length) {
  if (updating) return;

  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  tool.setLength(length * scale);

  update();
}


void ToolDialog::on_diameterDoubleSpinBox_valueChanged(double diameter) {
  if (updating) return;

  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;
  double angle = tool.getAngle();

  tool.setDiameter(diameter * scale);
  if (tool.getShape() == ToolShape::TS_CONICAL)
    tool.setLengthFromAngle(angle);

  update();
}


void ToolDialog::on_snubDiameterDoubleSpinBox_valueChanged(double value) {
  if (updating) return;

  double scale = tool.getUnits() == ToolUnits::UNITS_MM ? 1.0 : 25.4;

  tool.setSnubDiameter(value * scale);
  update();
}


void ToolDialog::on_descriptionLineEdit_textChanged(const QString &value) {
  if (updating) return;
  tool.setDescription(value.toUtf8().data());
  update();
}
