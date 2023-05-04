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
#include "ui_cam_layer_dialog.h"

#include <gcode/Units.h>

#include <cbang/json/JSON.h>

#include <QTableWidget>


namespace CAMotics {
  class CAMLayerDialog : public Dialog, public cb::JSON::Serializable {
    Q_OBJECT;
    CAMOTICS_DIALOG(CAMLayerDialog);

    bool metric;

  public:
    CAMLayerDialog(QWidget *parent);

    void setLayers(const std::vector<std::string> &layers);
    void setUnits(GCode::Units units);

    std::string getOffsetType(int index) const;
    std::string getOffsetType() const;
    void setOffsetType(const std::string &offset);

    void update();
    int exec();

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;

  protected slots:
    void on_offsetComboBox_currentIndexChanged(int) {update();}
    void on_startDepthDoubleSpinBox_valueChanged(double x);
    void on_endDepthDoubleSpinBox_valueChanged(double x);
  };
}
