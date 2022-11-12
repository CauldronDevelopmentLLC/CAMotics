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


namespace CAMotics {
  class Dialog : public QDialog {
  protected:
    class UIBase {
    public:
      virtual ~UIBase() {}
      virtual void setupUi(QDialog *dialog) = 0;
      virtual void retranslateUi(QDialog *dialog) = 0;
    };


    template <class T> class UI : public UIBase {
      cb::SmartPointer<T> ui;

    public:
      UI() : ui(new T) {}

      T &getUI() const {return *ui;}

      // From UIBase
      void setupUi(QDialog *dialog) {ui->setupUi(dialog);}
      void retranslateUi(QDialog *dialog) {ui->retranslateUi(dialog);}
    };

    cb::SmartPointer<UIBase> ui;

  public:
    Dialog(QWidget *parent, const cb::SmartPointer<UIBase> &ui,
           Qt::WindowFlags flags = Qt::WindowFlags()) :
      QDialog(parent, flags), ui(ui) {ui->setupUi(this);}

    template <class T> T &getUI() const {return ui.cast<UI<T> >()->getUI();}

    template <typename T> T &get(const std::string &name) const {
      T *widget = findChild<T *>(name.c_str());
      if (!widget) THROW("Could not find child '" << name << "'");
      return *widget;
    }

    bool isChecked(const std::string &name) const;

    cb::Vector3D getVector3D(const std::string &name) const;
    void setVector3D(const std::string &name, const cb::Vector3D &v) const;

    // From QDialog
    void changeEvent(QEvent *event);
  };
}
