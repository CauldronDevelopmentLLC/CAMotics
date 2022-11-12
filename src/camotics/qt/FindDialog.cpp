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

#include "FindDialog.h"

#include "ui_find_dialog.h"

#include <cbang/String.h>
#include <cbang/log/Logger.h>

#include <QTextDocument>

using namespace cb;
using namespace CAMotics;


#define UI() Dialog::getUI<Ui::FindDialog>()


FindDialog::FindDialog(QWidget *parent, bool replace) :
  Dialog(parent, new UI<Ui::FindDialog>), wasReplace(false) {
  if (!replace) {
    UI().replaceLabel->setVisible(false);
    UI().replaceLineEdit->setVisible(false);
    UI().replaceButton->setVisible(false);
    UI().replaceAllButton->setVisible(false);
  }

  connect(UI().replaceButton, SIGNAL(clicked()), this, SLOT(replace()));
  //  connect(UI().replaceButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(UI().replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAll()));
  //  connect(UI().replaceAllButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(UI().findButton, SIGNAL(clicked()), this, SLOT(find()));
  //  connect(UI().findButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(UI().cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void FindDialog::findReplace(bool find, bool all) {
  QString findText = UI().findLineEdit->text();
  QString replaceText = UI().replaceLineEdit->text();
  bool regex = UI().regexCheckBox->isChecked();
  int options = 0;

  if (findText.isEmpty()) return;

  if (UI().caseSensitiveCheckBox->isChecked())
    options |= QTextDocument::FindCaseSensitively;

  if (UI().wholeWordsCheckBox->isChecked())
    options |= QTextDocument::FindWholeWords;

  wasReplace = !find;

  if (find) emit this->find(findText, regex, options);
  else emit this->replace(findText, replaceText, regex, options, all);
}


void FindDialog::show() {
  UI().messageLabel->setText("");
  UI().replaceButton->setEnabled(false);
  UI().replaceAllButton->setEnabled(false);
  QDialog::show();
}


void FindDialog::find() {
  findReplace(true, false);
}


void FindDialog::findResult(bool found) {
  UI().messageLabel->setText((found || wasReplace) ? "" :
                            "<b><font color='red'>Not found!</font></b>");

  UI().replaceButton->setEnabled(found);
  UI().replaceAllButton->setEnabled(found);
}


void FindDialog::replace() {
  findReplace(false, false);
}


void FindDialog::replaceAll() {
  findReplace(false, true);
}
