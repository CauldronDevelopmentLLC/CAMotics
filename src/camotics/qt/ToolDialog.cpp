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

#include "ToolDialog.h"

#include <cbang/util/SmartToggle.h>

using namespace CAMotics;
using namespace cb;
using namespace std;


ToolDialog::ToolDialog(QWidget *parent) :
  Dialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint |
         Qt::WindowMaximizeButtonHint), updating(false) {
  ui.setupUi(this);
  ui.toolView->setScene(&scene);
}


bool ToolDialog::isMetric() const {
  return tool.getUnits() == GCode::Units::METRIC;
}


double ToolDialog::getScale() const {return isMetric() ? 1.0 : (1.0 / 25.4);}


int ToolDialog::edit() {
  update();
  return exec();
}


void ToolDialog::updateNumber() {
  ui.numberSpinBox->setValue((int)tool.getNumber());
}


void ToolDialog::updateUnits() {
  // ComboBox
  ui.unitsComboBox->setCurrentIndex(tool.getUnits());

  // Step
  const double step = isMetric() ? 1 : 0.125;
  ui.lengthDoubleSpinBox->setSingleStep(step);
  ui.diameterDoubleSpinBox->setSingleStep(step);
  ui.snubDiameterDoubleSpinBox->setSingleStep(step);

  // Suffix
  const char *suffix = isMetric() ? "mm" : "in";
  ui.lengthDoubleSpinBox->setSuffix(suffix);
  ui.diameterDoubleSpinBox->setSuffix(suffix);
  ui.snubDiameterDoubleSpinBox->setSuffix(suffix);
}


void ToolDialog::updateShape() {
  // ComboBox
  GCode::ToolShape shape = tool.getShape();
  ui.shapeComboBox->setCurrentIndex(shape);

  // Visibility
  ui.angleDoubleSpinBox->setVisible(shape == TS_CONICAL);
  ui.angleLabel->setVisible(shape == TS_CONICAL);
  ui.snubDiameterDoubleSpinBox-> setVisible(shape == TS_SNUBNOSE);
  ui.snubDiameterLabel->setVisible(shape == TS_SNUBNOSE);
}


void ToolDialog::updateAngle() {
  ui.angleDoubleSpinBox->setValue(tool.getAngle());
}


void ToolDialog::limitLength() {
  if (tool.getShape() == TS_BALLNOSE && tool.getLength() < tool.getRadius()) {
    tool.setLength(tool.getRadius() / getScale());
    updateLength();
  }
}


void ToolDialog::updateLength() {
  ui.lengthDoubleSpinBox->setValue(tool.getLength() * getScale());
}


void ToolDialog::updateDiameter() {
  ui.diameterDoubleSpinBox->setValue(tool.getDiameter() * getScale());
}


void ToolDialog::updateSnubDiameter() {
  ui.snubDiameterDoubleSpinBox->setValue(tool.getSnubDiameter() * getScale());
}


void ToolDialog::updateDescription() {
  ui.descriptionLineEdit->
    setText(QString::fromUtf8(tool.getDescription().c_str()));
}


void ToolDialog::updateScene() {scene.update(tool, ui.toolView->frameSize());}


void ToolDialog::update() {
  updateNumber();
  updateUnits();
  updateShape();
  updateAngle();
  updateLength();
  updateDiameter();
  updateSnubDiameter();
  updateDescription();
  updateScene();
}


void ToolDialog::resizeEvent(QResizeEvent *event) {
  QDialog::resizeEvent(event);
  updateScene();
}


void ToolDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  update();
}


void ToolDialog::on_numberSpinBox_valueChanged(int value) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setNumber((unsigned)value);
  update();
}


void ToolDialog::on_unitsComboBox_currentIndexChanged(int value) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setUnits((GCode::Units::enum_t)value);
  updateUnits();
  updateLength();
  updateDiameter();
  updateSnubDiameter();
  updateScene();
}


void ToolDialog::on_shapeComboBox_currentIndexChanged(int value) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setShape((GCode::ToolShape::enum_t)value);
  updateShape();
  limitLength();
  updateScene();
}


void ToolDialog::on_angleDoubleSpinBox_valueChanged(double angle) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setLengthFromAngle(angle);
  updateLength();
  updateScene();
}


void ToolDialog::on_lengthDoubleSpinBox_valueChanged(double length) {
  if (updating) return;
  SmartToggle toggle(updating);

  double angle = tool.getAngle(); // Get angle before length changes
  tool.setLength(length / getScale());
  limitLength();

  if (tool.getShape() == TS_CONICAL) {
    tool.setRadiusFromAngle(angle);
    updateDiameter();
  }

  updateScene();
}


void ToolDialog::on_diameterDoubleSpinBox_valueChanged(double diameter) {
  if (updating) return;
  SmartToggle toggle(updating);

  double angle = tool.getAngle(); // Get angle before diameter changes
  tool.setDiameter(diameter / getScale());

  if (tool.getShape() == TS_CONICAL) {
    tool.setLengthFromAngle(angle);
    updateLength();
  }

  updateScene();
}


void ToolDialog::on_snubDiameterDoubleSpinBox_valueChanged(double value) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setSnubDiameter(value / getScale());
  updateScene();
}


void ToolDialog::on_descriptionLineEdit_textChanged(const QString &value) {
  if (updating) return;
  SmartToggle toggle(updating);

  tool.setDescription(value.toUtf8().data());
  updateScene();
}
