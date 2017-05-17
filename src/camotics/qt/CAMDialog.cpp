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

#include "CAMDialog.h"

#include <camotics/dxf/DXFReader.h>

#include <cbang/log/Logger.h>

#include "ui_cam_dialog.h"


using namespace CAMotics;
using namespace std;


CAMDialog::CAMDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::CAMDialog), layerDialog(this), editRow(-1) {

  ui->setupUi(this);

  connect(&layerDialog, SIGNAL(accepted()),
          this, SLOT(layerDialogAccepted()));
}


void CAMDialog::loadDXFLayers(const string &filename) {
  DXFReader reader;
  reader.read(filename);

  const DXFReader::layers_t &layers = reader.getLayers();

  if (layers.empty()) THROW("DXF file has no layers");

  vector<string> names;
  DXFReader::layers_t::const_iterator it;
  for (it = layers.begin(); it != layers.end(); it++)
    names.push_back(it->first);

  layerDialog.setLayers(names);
}


void CAMDialog::setUnits(ToolUnits units) {
  bool metric = units == ToolUnits::UNITS_MM;

  ui->xTranslateDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui->yTranslateDoubleSpinBox->setSuffix(metric ? " mm" : " in");
  ui->safeDoubleSpinBox->setSuffix(metric ? " mm" : " in");

  layerDialog.setUnits(units);
}


int CAMDialog::getSelectedRow() const {
  QModelIndexList indexes =
    ui->camTableWidget->selectionModel()->selectedIndexes();
  if (indexes.empty()) return -1;
  return indexes.first().row();
}


void CAMDialog::on_addLayerPushButton_clicked() {
  editRow = -1;
  layerDialog.show();
}


void CAMDialog::on_upPushButton_clicked() {
  int row = getSelectedRow();
  if (row < 1) return;
  //ui->camTableWidget->verticalHeader()->moveSection(row, row - 1);
}


void CAMDialog::on_downPushButton_clicked() {
  int row = getSelectedRow();
  if (row == -1 || row == ui->camTableWidget->rowCount() - 1) return;
  //ui->camTableWidget->verticalHeader()->moveSection(row, row + 1);
}


void CAMDialog::on_camTableWidget_activated(QModelIndex index) {
  int row = index.row();
  if (row < 0) return;

  layerDialog.setLayer(layers[row]);
  editRow = row;
  layerDialog.show();
}


void CAMDialog::layerDialogAccepted() {
  CAMLayer layer = layerDialog.getLayer();
  QTableWidget *table = ui->camTableWidget;
  int row;

  if (editRow < 0) {
    row = table->rowCount();
    table->setRowCount(row + 1);
    layers.push_back(layer);

  } else {
    row = editRow;
    layers[row] = layer;
  }

  table->setItem(row, 0,
                 new QTableWidgetItem(QString().fromUtf8(layer.name.c_str())));
  table->setItem(row, 1, new QTableWidgetItem
                 (QString().sprintf("%d", layer.tool)));
  table->setItem(row, 2, new QTableWidgetItem
                 (QString().sprintf("%d", layer.feed)));
  table->setItem(row, 3, new QTableWidgetItem
                 (QString().sprintf("%d", layer.speed)));
  table->setItem(row, 4, new QTableWidgetItem
                 (layer.getOffsetString().c_str()));
  table->setItem(row, 5, new QTableWidgetItem
                 (QString()
                  .sprintf("%0.5g to %0.5g", layer.startDepth,
                           layer.endDepth)));
  table->setItem(row, 6, new QTableWidgetItem
                 (QString().sprintf("%0.5g", layer.maxStep)));
}
