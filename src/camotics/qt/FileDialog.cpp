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

#include "FileDialog.h"

#include "QtWin.h"

#include <cbang/os/SystemUtilities.h>
#include <cbang/log/Logger.h>

#include <QString>

using namespace CAMotics;
using namespace cb;
using namespace std;


FileDialog::FileDialog(QtWin &win) : win(win) {}


string FileDialog::open(const string &title, const string &filters,
                        const string &filename, bool save, bool anyFile) {
  // Find a resonable directory & file to start from
  QString qPath = QString::fromUtf8(filename.c_str());

  if (qPath.isEmpty()) {
    if (QFileInfo(QDir::currentPath()).isWritable())
      qPath = QDir::currentPath();
    else qPath = QDir::homePath();
  }

  QFileDialog dialog(&win, QString::fromUtf8(title.c_str()), qPath,
                     QString::fromUtf8(filters.c_str()));

  if (save || anyFile) dialog.setFileMode(QFileDialog::AnyFile);
  else dialog.setFileMode(QFileDialog::ExistingFile);

  dialog.setOption(QFileDialog::DontResolveSymlinks);
  if (!save) dialog.setOption(QFileDialog::DontConfirmOverwrite);

  if (save) dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (!save && anyFile) {
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setLabelText(QFileDialog::Accept, "Open");
  }

  if (!dialog.exec()) return "";

  QStringList files = dialog.selectedFiles();
  if (files.empty()) return "";

  qPath = files.first();

  if (QFileInfo(qPath).isDir()) {
    win.warning("Cannot open directory");
    return "";
  }

  return qPath.isEmpty() ? "" : qPath.toUtf8().data();
}
