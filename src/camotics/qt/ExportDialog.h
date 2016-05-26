/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef CAMOTICS_EXPORT_DIALOG_H
#define CAMOTICS_EXPORT_DIALOG_H

#include <cbang/SmartPointer.h>

#include <QDialog>


namespace Ui {class ExportDialog;}


namespace CAMotics {
  class ExportDialog : public QDialog {
    Q_OBJECT;

    cb::SmartPointer<Ui::ExportDialog> ui;

  public:
    ExportDialog();

    // From QDialog
    int exec();

    void enableSurface(bool enable);
    void enableGCode(bool enable);
    void enableSimData(bool enable);

    bool surfaceSelected() const;
    bool gcodeSelected() const;
    bool simDataSelected() const;

    bool binarySTLSelected() const;
    bool compactJSONSelected() const;

  protected slots:
    void on_surfaceRadioButton_clicked();
    void on_gcodeRadioButton_clicked();
    void on_simDataRadioButton_clicked();
  };
}

#endif // CAMOTICS_EXPORT_DIALOG_H
