/******************************************************************************\

             CAMotics is an Open-Source simulation and CAM software.
     Copyright (C) 2011-2021 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#include "Dialog.h"

#include <QEvent>
#include <QDoubleSpinBox>
#include <QAbstractButton>


using namespace std;
using namespace cb;
using namespace CAMotics;


bool Dialog::isChecked(const string &name) const {
  return get<QAbstractButton>(name).isChecked();
}


Vector3D Dialog::getVector3D(const string &name) const {
  Vector3D result;
  const char *axes[] = {"X", "Y", "Z"};

  for (unsigned i = 0; i < 3; i++)
    result[i] = get<QDoubleSpinBox>(name + axes[i] + "DoubleSpinBox").value();

  return result;
}


void Dialog::setVector3D(const string &name, const Vector3D &v) const {
  const char *axes[] = {"X", "Y", "Z"};

  for (unsigned i = 0; i < 3; i++)
    get<QDoubleSpinBox>(name + axes[i] + "DoubleSpinBox").setValue(v[i]);
}


void Dialog::changeEvent(QEvent *event) {
  if (event->type() == QEvent::LanguageChange) retranslateUi();
  else QDialog::changeEvent(event);
}
