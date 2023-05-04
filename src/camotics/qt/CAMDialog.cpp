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

#include "CAMDialog.h"

#include <dxf/Reader.h>

#include <cbang/log/Logger.h>


using namespace CAMotics;
using namespace cb;
using namespace std;


CAMDialog::CAMDialog(QWidget *parent) :
  Dialog(parent), layerDialog(this), metric(true), layers(new JSON::List),
  editRow(-1) {
  ui.setupUi(this);
  connect(&layerDialog, SIGNAL(accepted()), this, SLOT(layerDialogAccepted()));
}


void CAMDialog::loadDXFLayers(const string &filename) {
  DXF::Reader reader;
  reader.read(filename);

  const DXF::Reader::layers_t &layers = reader.getLayers();

  if (layers.empty()) THROW("DXF file has no layers");

  vector<string> names;
  DXF::Reader::layers_t::const_iterator it;
  for (it = layers.begin(); it != layers.end(); it++)
    names.push_back(it->first);

  layerDialog.setLayers(names);
}


void CAMDialog::setUnits(GCode::Units units) {
  metric = units == GCode::Units::METRIC;

  ui.xTranslateDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui.yTranslateDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui.safeDoubleSpinBox->setSuffix(metric ? " mm" : " in");

  layerDialog.setUnits(units);
}


int CAMDialog::getSelectedRow() const {
  QModelIndexList indexes =
    ui.camTableWidget->selectionModel()->selectedIndexes();
  if (indexes.empty()) return -1;
  return indexes.first().row();
}


void CAMDialog::read(const JSON::Value &value) {
  double scale = metric ? 1 : 25.4;

  if (value.hasDict("translate")) {
    auto &d = value.getDict("translate");
    ui.xTranslateDoubleSpinBox->setValue(d.getNumber("x", 0) * scale);
    ui.yTranslateDoubleSpinBox->setValue(d.getNumber("y", 0) * scale);
  }

  if (value.hasDict("scale")) {
    auto &d = value.getDict("scale");
    ui.xScaleDoubleSpinBox->setValue(d.getNumber("x", 0));
    ui.yScaleDoubleSpinBox->setValue(d.getNumber("y", 0));
  }

  if (value.hasDict("shrink")) {
    auto &d = value.getDict("shrink");
    ui.xShrinkCheckBox->setChecked(d.getBoolean("x", false));
    ui.yShrinkCheckBox->setChecked(d.getBoolean("y", false));
  }

  if (value.hasDict("flip")) {
    auto &d = value.getDict("flip");
    ui.xFlipCheckBox->setChecked(d.getBoolean("x", false));
    ui.yFlipCheckBox->setChecked(d.getBoolean("y", false));
  }

  if (value.hasDict("array")) {
    auto &d = value.getDict("array");
    ui.xArraySpinBox->setValue(d.getNumber("x", 0));
    ui.yArraySpinBox->setValue(d.getNumber("y", 0));
  }

  ui.safeDoubleSpinBox->setValue(value.getNumber("safe-height", 0) * scale);

  layers = value.get("layers", layers);
}


void CAMDialog::write(JSON::Sink &sink) const {
  double scale = metric ? 1 : 25.4;

  sink.beginDict();

  sink.insertDict("translate");
  sink.insert("x", ui.xTranslateDoubleSpinBox->value() / scale);
  sink.insert("y", ui.yTranslateDoubleSpinBox->value() / scale);
  sink.endDict();

  sink.insertDict("scale");
  sink.insert("x", ui.xScaleDoubleSpinBox->value());
  sink.insert("y", ui.yScaleDoubleSpinBox->value());
  sink.endDict();

  sink.insertDict("shrink");
  sink.insertBoolean("x", ui.xShrinkCheckBox->isChecked());
  sink.insertBoolean("y", ui.yShrinkCheckBox->isChecked());
  sink.endDict();

  sink.insertDict("flip");
  sink.insertBoolean("x", ui.xFlipCheckBox->isChecked());
  sink.insertBoolean("y", ui.yFlipCheckBox->isChecked());
  sink.endDict();

  sink.insertDict("array");
  sink.insert("x", ui.xArraySpinBox->value());
  sink.insert("y", ui.yArraySpinBox->value());
  sink.endDict();

  sink.insert("safe-height", ui.safeDoubleSpinBox->value() / scale);

  sink.insert("layers", *layers);

  sink.endDict();
}


void CAMDialog::on_addLayerPushButton_clicked() {
  editRow = -1;
  layerDialog.show();
}


void CAMDialog::on_upPushButton_clicked() {
  int row = getSelectedRow();
  if (row < 1) return;
  //ui.camTableWidget->verticalHeader()->moveSection(row, row - 1);
}


void CAMDialog::on_downPushButton_clicked() {
  int row = getSelectedRow();
  if (row == -1 || row == ui.camTableWidget->rowCount() - 1) return;
  //ui.camTableWidget->verticalHeader()->moveSection(row, row + 1);
}


void CAMDialog::on_camTableWidget_activated(QModelIndex index) {
  int row = index.row();
  if (row < 0) return;

  layerDialog.read(layers->getDict(row));
  editRow = row;
  layerDialog.show();
}


void CAMDialog::setRow(int row, int col, const string &text) {
  const QString s = QString().fromUtf8(text.c_str());
  ui.camTableWidget->setItem(row, col, new QTableWidgetItem(s));
}


void CAMDialog::layerDialogAccepted() {
  SmartPointer<JSON::Value> layer = layerDialog.toJSON();
  QTableWidget *table = ui.camTableWidget;
  int row;

  if (editRow < 0) {
    row = table->rowCount();
    table->setRowCount(row + 1);
    layers->append(layer);

  } else {
    row = editRow;
    layers->set(row, layer);
  }

  setRow(row, 0, layer->getString("name"));
  setRow(row, 1, layer->getAsString("tool", "0"));
  setRow(row, 2, layer->getAsString("feed", "0"));
  setRow(row, 3, layer->getAsString("speed", "0"));
  setRow(row, 4, layer->getAsString("offset-type", "0"));
  setRow(row, 5, String::printf("%0.5g to %0.5g",
                                layer->getNumber("start-depth", 0),
                                layer->getNumber("end-depth", 0)));
  setRow(row, 6, String::printf("%0.5g", layer->getNumber("max-step-down", 0)));
}
