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

#include "AboutDialog.h"

#include "ui_about_dialog.h"

#include <cbang/Info.h>

#include <QMessageBox>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


AboutDialog::AboutDialog() : ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  // Set version
  string version = "Version " + Info::instance().get("Build", "Version");
  ui->versionLabel->setText(version.c_str());
}


void AboutDialog::on_creditsPushButton_clicked() {
  Info &info = Info::instance();
  string credits = info.get("OpenSCAM", "Author") + "\n" +
    info.get("OpenSCAM", "Organization");


  QMessageBox::information
    (this, "OpenSCAM - Credits", credits.c_str(), QMessageBox::Ok);
}


void AboutDialog::on_licensePushButton_clicked() {
  QMessageBox::information
    (this, "OpenSCAM - License",
     "This program is free software: you can redistribute it and/or modify it "
     "under the terms of the GNU General Public License as published by the "
     "Free Software Foundation, either version 2 of the License, or (at your "
     "option) any later version.\n"
     "\n"
     "This program is distributed in the hope that it will be useful, but "
     "WITHOUT ANY WARRANTY; without even the implied warranty of "
     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
     "General Public License for more details.\n"
     "\n"
     "You should have received a copy of the GNU General Public License "
     "along with this program.  If not, see <http://www.gnu.org/licenses/>.",
     QMessageBox::Ok);
}
