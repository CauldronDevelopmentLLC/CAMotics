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

#ifndef CAMOTICS_FILE_TAB_MANAGER_H
#define CAMOTICS_FILE_TAB_MANAGER_H

#include <cbang/SmartPointer.h>

#include <QTabWidget>


namespace CAMotics {
  class QtWin;
  class FileDialog;
  class NCFile;
  class NCEdit;


  class FileTabManager : public QTabWidget {
    Q_OBJECT;

    QtWin *win;
    unsigned offset;

  public:
    FileTabManager(QWidget *parent = 0);

    void open(const cb::SmartPointer<NCFile> &file,
              int line = -1, int col = -1);
    bool isFileTab(unsigned tab) const {return offset <= tab;}
    bool isModified(unsigned tab) const;
    const cb::SmartPointer<NCFile> &getFile(unsigned tab) const;
    bool checkSave(unsigned tab);
    bool checkSaveAll();
    void save();
    void saveAs();
    void save(unsigned tab, bool saveAs = false);
    void saveAll();
    void revert();
    void revert(unsigned tab);
    void revertAll();
    void close(unsigned tab, bool canSave = true, bool removeTab = true);
    void closeAll(bool canSave = true, bool removeTab = true);

    void validateTabIndex(unsigned tab) const;
    NCEdit *getEditor(unsigned tab) const;
    NCEdit *getCurrentEditor() const;

  signals:
    void find();
    void findNext();
    void findResult(bool);

  protected slots:
    void on_tabCloseRequested(int index);

    void on_modificationChanged(bool changed);

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();

    void on_find(QString find, bool regex, int options);
    void on_replace(QString find, QString replace, bool regex, int options,
                    bool all);
  };
}

#endif // CAMOTICS_FILE_TAB_MANAGER_H
