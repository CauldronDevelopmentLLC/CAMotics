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

#include "CAMLayerDialog.h"
#include "ui_cam_dialog.h"


namespace CAMotics {
  class CAMDialog : public Dialog, public cb::JSON::Serializable {
    Q_OBJECT;
    CAMOTICS_DIALOG(CAMDialog);

    CAMLayerDialog layerDialog;
    bool metric;
    cb::SmartPointer<cb::JSON::Value> layers;
    int editRow;

  public:
    CAMDialog(QWidget *parent);

    void loadDXFLayers(const std::string &filename);
    void setUnits(GCode::Units units);

    int getSelectedRow() const;

    // From cb::JSON::Serializable
    void read(const cb::JSON::Value &value);
    void write(cb::JSON::Sink &sink) const;

  protected slots:
    void on_addLayerPushButton_clicked();
    void on_upPushButton_clicked();
    void on_downPushButton_clicked();
    void on_camTableWidget_activated(QModelIndex index);

    void setRow(int row, int col, const std::string &text);
    void layerDialogAccepted();
  };
}
