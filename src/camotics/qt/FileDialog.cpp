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

#include "FileDialog.h"

#include "QtWin.h"

#include <cbang/os/SystemUtilities.h>
#include <cbang/log/Logger.h>

#include <QString>
#include <QObject>

using namespace CAMotics;
using namespace cb;
using namespace std;


FileDialog::FileDialog(QtWin &win) : win(win) {}


QString FileDialog::open(const QString &title, const QString &filters,
                         const QString &filename, bool save, bool anyFile) {
  // Find a resonable directory & file to start from
  QString qPath = filename;

  if (qPath.isEmpty()) {
    if (QFileInfo(QDir::currentPath()).isWritable())
      qPath = QDir::currentPath();
    else qPath = QDir::homePath();
  }

  QFileDialog dialog(&win, title, qPath, filters);

  if (save || anyFile) dialog.setFileMode(QFileDialog::AnyFile);
  else dialog.setFileMode(QFileDialog::ExistingFile);

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
    win.warning(QObject::tr("Cannot open directory"));
    return "";
  }

  return qPath;
}
