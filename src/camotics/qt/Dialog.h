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

#pragma once

#include <cbang/SmartPointer.h>
#include <cbang/geom/Vector.h>

#include <QDialog>


#define CAMOTICS_DIALOG(NAME)                   \
  Ui::NAME ui;                                  \
  void retranslateUi() {ui.retranslateUi(this);}


namespace CAMotics {
  class Dialog : public QDialog {
 public:
    Dialog(QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags()) :
      QDialog(parent, flags) {}
    virtual ~Dialog() {}

    template <typename T> T &get(const std::string &name) const {
      T *widget = findChild<T *>(name.c_str());
      if (!widget) THROW("Could not find child '" << name << "'");
      return *widget;
    }

    bool isChecked(const std::string &name) const;

    cb::Vector3D getVector3D(const std::string &name) const;
    void setVector3D(const std::string &name, const cb::Vector3D &v) const;

    virtual void retranslateUi() {};
    
    // From QDialog
    void changeEvent(QEvent *event);
  };
}
