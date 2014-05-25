/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

#ifndef OPENSCAM_FILE_TAB_MANAGER_H
#define OPENSCAM_FILE_TAB_MANAGER_H

#include <cbang/SmartPointer.h>

#include <QTabWidget>


namespace OpenSCAM {
  class QtWin;
  class FileDialog;
  class NCFile;
  class NCEdit;


  class FileTabManager : public QObject {
    Q_OBJECT;

    QtWin &win;
    QTabWidget &tabWidget;
    unsigned offset;

  public:
    FileTabManager(QtWin &win, QTabWidget &tabWidget, unsigned offset);

    void open(const cb::SmartPointer<NCFile> &file);
    bool isModified(unsigned tab) const;
    const cb::SmartPointer<NCFile> &getFile(unsigned tab) const;
    bool checkSave(unsigned tab);
    void save(unsigned tab, bool saveAs = false);
    void saveAll();
    void revert(unsigned tab);
    void revertAll();
    void close(unsigned tab, bool canSave = true, bool removeTab = true);
    void closeAll(bool canSave = true, bool removeTab = true);

    void validateTabIndex(unsigned tab) const;
    NCEdit *getEditor(unsigned tab) const;
    NCEdit *getCurrentEditor() const;

  protected slots:
    void on_modificationChanged(bool changed);

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();
  };
}

#endif // OPENSCAM_FILE_TAB_MANAGER_H

