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

#include <cbang/String.h>
#include <cbang/log/Logger.h>

#include <QTextDocument>

using namespace cb;
using namespace CAMotics;


FindDialog::FindDialog(QWidget *parent, bool replace) :
  Dialog(parent), wasReplace(false) {
  ui.setupUi(this);
  
  if (!replace) {
    ui.replaceLabel    ->setVisible(false);
    ui.replaceLineEdit ->setVisible(false);
    ui.replaceButton   ->setVisible(false);
    ui.replaceAllButton->setVisible(false);
  }

  connect(ui.replaceButton,    SIGNAL(clicked()), this, SLOT(replace()));
  connect(ui.replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAll()));
  connect(ui.findButton,       SIGNAL(clicked()), this, SLOT(find()));
  connect(ui.cancelButton,     SIGNAL(clicked()), this, SLOT(reject()));
}


void FindDialog::findReplace(bool find, bool all) {
  QString findText    = ui.findLineEdit->text();
  QString replaceText = ui.replaceLineEdit->text();
  bool    regex       = ui.regexCheckBox->isChecked();
  int     options     = 0;

  if (findText.isEmpty()) return;

  if (ui.caseSensitiveCheckBox->isChecked())
    options |= QTextDocument::FindCaseSensitively;

  if (ui.wholeWordsCheckBox->isChecked())
    options |= QTextDocument::FindWholeWords;

  wasReplace = !find;

  if (find) emit this->find(findText, regex, options);
  else emit this->replace(findText, replaceText, regex, options, all);
}


void FindDialog::show() {
  ui.messageLabel->setText("");
  ui.replaceButton->setEnabled(false);
  ui.replaceAllButton->setEnabled(false);
  QDialog::show();
}


void FindDialog::find() {findReplace(true, false);}


void FindDialog::findResult(bool found) {
  ui.messageLabel->setText((found || wasReplace) ? "" :
                            "<b><font color='red'>Not found!</font></b>");

  ui.replaceButton->setEnabled(found);
  ui.replaceAllButton->setEnabled(found);
}


void FindDialog::replace()    {findReplace(false, false);}
void FindDialog::replaceAll() {findReplace(false, true);}
