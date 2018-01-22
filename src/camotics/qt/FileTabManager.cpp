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

#include "FileTabManager.h"

#include "QtWin.h"
#include "NCEdit.h"
#include "GCodeHighlighter.h"
#include "TPLHighlighter.h"

#include <camotics/sim/NCFile.h>

#include <cbang/Exception.h>
#include <cbang/os/SystemUtilities.h>

#include <QFile>
#include <QString>
#include <QMessageBox>

using namespace CAMotics;
using namespace cb;
using namespace std;


FileTabManager::FileTabManager(QWidget *parent) :
  QTabWidget(parent), win(0), offset(1) {

  while (parent && !win) {
    win = dynamic_cast<QtWin *>(parent);
    parent = parent->parentWidget();
  }

  if (!win) THROW("QtWin not found");

  connect(this, SIGNAL(tabCloseRequested(int)),
          SLOT(on_tabCloseRequested(int)));
}


void FileTabManager::open(const SmartPointer<NCFile> &file,
                          int line, int col) {
  // Check if we already have this file open in a tab
  unsigned tab;
  for (tab = offset; tab < (unsigned)QTabWidget::count(); tab++)
    if (getFile(tab) == file) break;

  // Create new tab
  if ((unsigned)QTabWidget::count() <= tab) {
    string absPath = file->getAbsolutePath();

    bool isTPL = String::endsWith(absPath, ".tpl");
    SmartPointer<Highlighter> highlighter;
    if (isTPL) highlighter = new TPLHighlighter;
    else highlighter = new GCodeHighlighter;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    NCEdit *editor = new NCEdit(file, highlighter, this);

    QFile qFile(QString::fromUtf8(absPath.c_str()));
    qFile.open(QFile::ReadOnly);
    QString contents = qFile.readAll();
    qFile.close();
    contents.replace('\t', "  ");

    editor->loadDarkScheme();
    editor->setWordWrapMode(QTextOption::NoWrap);
    editor->setPlainText(contents);

    connect(editor, SIGNAL(find()), SIGNAL(find()));
    connect(editor, SIGNAL(findNext()), SIGNAL(findNext()));
    connect(editor, SIGNAL(findResult(bool)), SIGNAL(findResult(bool)));

    QString title = QString::fromUtf8
      (SystemUtilities::basename(file->getAbsolutePath()).c_str());
    tab = (unsigned)QTabWidget::addTab(editor, title);
    QApplication::restoreOverrideCursor();
  }

  // Get editor
  NCEdit *editor = (NCEdit *)QTabWidget::widget(tab);

  // Switch to tab
  QTabWidget::setCurrentIndex(tab);

  // Select line and column
  if (0 < line) {
    QTextCursor c = editor->textCursor();

    c.setPosition(0, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);

    if (0 < col)
      c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, col);

    c.select(QTextCursor::LineUnderCursor);

    editor->setTextCursor(c);
  }

  // Get input focus
  editor->setFocus();
}


bool FileTabManager::isModified(unsigned tab) const {
  validateTabIndex(tab);
  return QTabWidget::tabText(tab).endsWith(" *");
}


const SmartPointer<NCFile> &FileTabManager::getFile(unsigned tab) const {
  validateTabIndex(tab);
  return ((NCEdit *)QTabWidget::widget(tab))->getFile();
}


bool FileTabManager::checkSave(unsigned tab) {
  if (!isModified(tab)) return true;

  // Select tab
  QTabWidget::setCurrentIndex(tab);

  int response =
    QMessageBox::question(this, "File Modified", "The current file has "
                          "been modifed.  Would you like to save it?",
                          QMessageBox::Cancel | QMessageBox::No |
                          QMessageBox::Yes, QMessageBox::Yes);

  if (response == QMessageBox::Yes) save(tab);
  else if (response == QMessageBox::Cancel) return false;
  return true;
}


bool FileTabManager::checkSaveAll() {
  bool all = false;

  try {
    for (int tab = offset; tab < QTabWidget::count(); tab++)
      if (isModified(tab)) {
        if (!all) {
          // Select tab
          QTabWidget::setCurrentIndex(tab);

          int response =
            QMessageBox::question(this, "File Modified", "The current file has "
                                  "been modifed.  Would you like to save?",
                                  QMessageBox::Cancel | QMessageBox::No |
                                  QMessageBox::Save | QMessageBox::SaveAll,
                                  QMessageBox::SaveAll);

          if (response == QMessageBox::SaveAll) all = true;
          if (response == QMessageBox::No) continue;
          if (response == QMessageBox::Cancel) return false;
        }

        save(tab);
      }

    return true;

  } catch (const Exception &e) {
    win->warning("Failed to save: " + e.getMessage());
  }

  return false;
}


void FileTabManager::save() {
  save(currentIndex(), false);
}


void FileTabManager::saveAs() {
  save(currentIndex(), true);
}


void FileTabManager::save(unsigned tab, bool saveAs) {
  validateTabIndex(tab);
  if (!saveAs && !isModified(tab)) return;

  // Get absolute path
  NCEdit *editor = (NCEdit *)QTabWidget::widget(tab);
  NCFile &file = *editor->getFile();
  string filename = file.getAbsolutePath();

  // Get type
  bool tpl = editor->isTPL();

  if (saveAs) {
    filename = win->openFile("Save file", tpl ? "TPL (*.tpl)" :
                             "GCode (*.nc *.ngc *.gcode)", filename, true);
    if (filename.empty()) return;

    string ext = SystemUtilities::extension(filename);
    if (ext.empty()) filename += tpl ? ".tpl" : ".gcode";

    else if (tpl && ext != "tpl") {
      win->warning("TPL file must have .tpl extension");
      return;

    } else if (!tpl && (ext == "xml" || ext == "tpl")) {
      win->warning("GCode file cannot have .tpl or .xml extension");
      return;
    }
  }

  // Save data
  QString content = editor->toPlainText();
  QFile qFile(QString::fromUtf8(filename.c_str()));
  if (!qFile.open(QFile::WriteOnly | QIODevice::Truncate))
    THROWS("Could not save '" << filename << "'");
  qFile.write(content.toUtf8());
  qFile.close();

  // Update file name
  filename = SystemUtilities::absolute(filename);
  if (filename != file.getAbsolutePath()) {
    file.setFilename(filename);

    // Update tab title
    QString
      title(QString::fromUtf8(SystemUtilities::basename(filename).c_str()));
    QTabWidget::setTabText(tab, title);
  }

  // Set unmodified
  editor->document()->setModified(false);

  // Notify
  win->showMessage("Saved " + file.getRelativePath());
}


void FileTabManager::saveAll() {
  for (int tab = offset; tab < QTabWidget::count(); tab++) save(tab);
}


void FileTabManager::revert() {
  revert(currentIndex());
}


void FileTabManager::revert(unsigned tab) {
  // Check if modified
  if (!isModified(tab)) return;

  // Get absolute path
  NCEdit *editor = (NCEdit *)QTabWidget::widget(tab);
  NCFile &file = *editor->getFile();
  string filename = file.getAbsolutePath();

  if (!SystemUtilities::exists(filename)) return;

  // Read data
  QFile qFile(filename.c_str());
  qFile.open(QFile::ReadOnly);
  QString contents = qFile.readAll();
  qFile.close();
  contents.replace('\t', " ");

  // Revert
  editor->setPlainText(contents);

  // Set unmodified
  editor->document()->setModified(false);

  // Notify
  win->showMessage("Reverted " + file.getRelativePath());
}


void FileTabManager::revertAll() {
  for (int tab = offset; tab < QTabWidget::count(); tab++) revert(tab);
}


void FileTabManager::close(unsigned tab, bool canSave, bool removeTab) {
  if (canSave && !checkSave(tab)) return;
  delete (NCEdit *)QTabWidget::widget(tab);
  if (removeTab) QTabWidget::removeTab(tab);
}


void FileTabManager::closeAll(bool canSave, bool removeTab) {
  for (int tab = offset; tab < QTabWidget::count(); tab++)
    close(tab, canSave, removeTab);
}


void FileTabManager::validateTabIndex(unsigned tab) const {
  if (tab < offset || (unsigned)QTabWidget::count() <= tab)
    THROWS("Invalid file tab index " << tab);
}


NCEdit *FileTabManager::getEditor(unsigned tab) const {
  validateTabIndex(tab);
  return (NCEdit *)QTabWidget::widget(tab);
}


NCEdit *FileTabManager::getCurrentEditor() const {
  return getEditor(QTabWidget::currentIndex());
}


int FileTabManager::getEditorIndex(NCEdit *editor) const {
  for (int i = offset; i < QTabWidget::count(); i++)
    if (QTabWidget::widget(i) == editor) return i;

  return -1;
}


void FileTabManager::on_modificationChanged(NCEdit *editor, bool changed) {
  int tab = getEditorIndex(editor);
  if (tab == -1) return;
  QString title = QTabWidget::tabText(tab);

  if (changed && !title.endsWith(" *"))
    QTabWidget::setTabText(tab, title + tr(" *"));

  else if (!changed && title.endsWith(" *"))
    QTabWidget::setTabText(tab, title.left(title.size() - 2));

  win->updateActions();
}


void FileTabManager::on_tabCloseRequested(int index) {
  close(index, true, false);
}


void FileTabManager::on_actionUndo_triggered() {getCurrentEditor()->undo();}
void FileTabManager::on_actionRedo_triggered() {getCurrentEditor()->redo();}
void FileTabManager::on_actionCut_triggered() {getCurrentEditor()->cut();}
void FileTabManager::on_actionCopy_triggered() {getCurrentEditor()->copy();}
void FileTabManager::on_actionPaste_triggered() {getCurrentEditor()->paste();}


void FileTabManager::on_actionSelectAll_triggered() {
  getCurrentEditor()->selectAll();
}


void FileTabManager::on_find(QString find, bool regex, int options) {
  getCurrentEditor()->find(find, regex, options);
}


void FileTabManager::on_replace(QString find, QString replace, bool regex,
                                int options, bool all) {
  getCurrentEditor()->replace(find, replace, regex, options, all);
}
