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

#include "NewProjectDialog.h"

using namespace CAMotics;


NewProjectDialog::NewProjectDialog(QWidget *parent) :
  Dialog(parent) {ui.setupUi(this);}


void NewProjectDialog::setUnits(GCode::Units units) {
  ui.unitsComboBox->setCurrentIndex
    (units == GCode::Units::METRIC ? 0 : 1);
}


GCode::Units NewProjectDialog::getUnits() const {
  return ui.unitsComboBox->currentIndex() == 0 ?
    GCode::Units::METRIC : GCode::Units::IMPERIAL;
}


bool NewProjectDialog::defaultToolTableSelected() const {
  return ui.toolTableComboBox->currentIndex() == 0;
}


bool NewProjectDialog::currentToolTableSelected() const {
  return ui.toolTableComboBox->currentIndex() == 1;
}
