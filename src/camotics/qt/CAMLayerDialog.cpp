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

#include "CAMLayerDialog.h"

#include "ui_cam_layer_dialog.h"

using namespace CAMotics;
using namespace std;


CAMLayerDialog::CAMLayerDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::CAMLayerDialog) {
  ui->setupUi(this);
}


void CAMLayerDialog::setLayers(const vector<string> &layers) {
  ui->layerComboBox->clear();
  for (unsigned i = 0; i < layers.size(); i++)
    ui->layerComboBox->addItem(QString::fromUtf8(layers[i].c_str()));
}


void CAMLayerDialog::setUnits(ToolUnits units) {
  bool metric = units == ToolUnits::UNITS_MM;

  ui->feedSpinBox->setSuffix(metric ? " mm/min" : " in/min");
  ui->offsetDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui->startDepthDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui->endDepthDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui->stepDoubleSpinBox->setSuffix(metric ? " mm" : " in");
}


CAMLayer CAMLayerDialog::getLayer() const {
  return CAMLayer(ui->layerComboBox->currentText().toUtf8().data(),
                  ui->toolSpinBox->value(),
                  ui->feedSpinBox->value(),
                  ui->speedSpinBox->value(),
                  (OffsetType::enum_t)ui->offsetComboBox->currentIndex(),
                  ui->offsetDoubleSpinBox->value(),
                  ui->startDepthDoubleSpinBox->value(),
                  ui->endDepthDoubleSpinBox->value(),
                  ui->stepDoubleSpinBox->value());
}


void CAMLayerDialog::setLayer(const CAMLayer &layer) {
  int index =
    ui->layerComboBox->findText(QString().fromUtf8(layer.name.c_str()));
  if (index != -1) ui->layerComboBox->setCurrentIndex(index);

  ui->toolSpinBox->setValue(layer.tool);
  ui->feedSpinBox->setValue(layer.feed);
  ui->speedSpinBox->setValue(layer.speed);
  ui->offsetComboBox->setCurrentIndex(layer.offsetType);
  ui->offsetDoubleSpinBox->setValue(layer.offset);
  ui->startDepthDoubleSpinBox->setValue(layer.startDepth);
  ui->endDepthDoubleSpinBox->setValue(layer.endDepth);
  ui->stepDoubleSpinBox->setValue(layer.maxStep);
}


void CAMLayerDialog::update() {
  bool manual = ui->offsetComboBox->currentIndex() == 3;
  ui->offsetDoubleSpinBox->setEnabled(manual);
}


int CAMLayerDialog::exec() {
  update();
  return QDialog::exec();
}


void CAMLayerDialog::on_startDepthDoubleSpinBox_valueChanged(double x) {
  if (x < ui->endDepthDoubleSpinBox->value())
    ui->endDepthDoubleSpinBox->setValue(x);
}


void CAMLayerDialog::on_endDepthDoubleSpinBox_valueChanged(double x) {
  if (ui->startDepthDoubleSpinBox->value() < x)
    ui->startDepthDoubleSpinBox->setValue(x);
}
