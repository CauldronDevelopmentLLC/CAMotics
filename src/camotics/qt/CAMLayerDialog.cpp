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

#include "CAMLayerDialog.h"

using namespace CAMotics;
using namespace cb;
using namespace std;


CAMLayerDialog::CAMLayerDialog(QWidget *parent) :
  Dialog(parent), metric(true) {ui.setupUi(this);}


void CAMLayerDialog::setLayers(const vector<string> &layers) {
  ui.layerComboBox->clear();
  for (unsigned i = 0; i < layers.size(); i++)
    ui.layerComboBox->addItem(QString::fromUtf8(layers[i].c_str()));
}


void CAMLayerDialog::setUnits(GCode::Units units) {
  metric = units == GCode::Units::METRIC;

  ui.feedSpinBox->setSuffix(metric ? " mm/min" : " in/min");
  ui.offsetDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui.startDepthDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui.endDepthDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui.stepDoubleSpinBox->setSuffix(metric ? " mm" : " in");
}


string CAMLayerDialog::getOffsetType(int index) const {
  return String::toLower(ui.offsetComboBox->itemText(index).toUtf8().data());
}


string CAMLayerDialog::getOffsetType() const {
  return getOffsetType(ui.offsetComboBox->currentIndex());
}


void CAMLayerDialog::setOffsetType(const string &offset) {
  for (int i = 0; i < ui.offsetComboBox->count(); i++)
    if (offset == getOffsetType(i)) ui.offsetComboBox->setCurrentIndex(i);
}


void CAMLayerDialog::update() {
  ui.offsetDoubleSpinBox->setEnabled(getOffsetType() == "custom");
}


int CAMLayerDialog::exec() {
  update();
  return QDialog::exec();
}


void CAMLayerDialog::read(const JSON::Value &value) {
  const double scale = metric ? 1 : 25.4;

  ui.toolSpinBox->setValue(value.getNumber("tool", 0));
  ui.feedSpinBox->setValue(value.getNumber("feed", 0) * scale);
  ui.speedSpinBox->setValue(value.getNumber("speed", 0));
  setOffsetType(value.getString("offset-type", "none"));
  ui.offsetDoubleSpinBox->setValue(value.getNumber("offset", 0));
  ui.startDepthDoubleSpinBox->
    setValue(value.getNumber("start-depth", 0) * scale);
  ui.endDepthDoubleSpinBox->setValue(value.getNumber("end-depth", 0) * scale);
  ui.stepDoubleSpinBox->setValue(value.getNumber("max-step-down", 0) * scale);
}


void CAMLayerDialog::write(JSON::Sink &sink) const {
  const double scale = metric ? 1 : 25.4;

  sink.beginDict();
  sink.insert("name", ui.layerComboBox->currentText().toUtf8().data());
  sink.insert("tool", (int)ui.toolSpinBox->value());
  sink.insert("feed", ui.feedSpinBox->value() / scale);
  sink.insert("speed", ui.speedSpinBox->value());
  sink.insert("offset-type", getOffsetType());
  if (getOffsetType() == "custom")
    sink.insert("offset", ui.offsetDoubleSpinBox->value());
  sink.insert("start-depth", ui.startDepthDoubleSpinBox->value() / scale);
  sink.insert("end-depth", ui.endDepthDoubleSpinBox->value() / scale);
  sink.insert("max-step-down", ui.stepDoubleSpinBox->value() / scale);
  sink.endDict();
}


void CAMLayerDialog::on_startDepthDoubleSpinBox_valueChanged(double x) {
  if (x < ui.endDepthDoubleSpinBox->value())
    ui.endDepthDoubleSpinBox->setValue(x);
}


void CAMLayerDialog::on_endDepthDoubleSpinBox_valueChanged(double x) {
  if (ui.startDepthDoubleSpinBox->value() < x)
    ui.startDepthDoubleSpinBox->setValue(x);
}
