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
                        const string &filename, bool save) {
  // Find a resonable directory & file to start from
  QString qPath = QString::fromUtf8(filename.c_str());

  if (qPath.isEmpty()) {
    if (QFileInfo(QDir::currentPath()).isWritable())
      qPath = QDir::currentPath();
    else qPath = QDir::homePath();
  }

  QString qTitle(QString::fromUtf8(title.c_str()));
  QString qFilters(QString::fromUtf8(filters.c_str()));
  QFlags<QFileDialog::Option> qOptions(QFileDialog::DontResolveSymlinks);
  if (!save) qOptions |= QFileDialog::DontConfirmOverwrite;

  qPath = (save ? QFileDialog::getSaveFileName : QFileDialog::getOpenFileName)
    (&win, qTitle, qPath, qFilters, 0, qOptions);

  if (QFileInfo(qPath).isDir()) {
    win.warning("Cannot open directory");
    return "";
  }

  return qPath.isEmpty() ? "" : qPath.toUtf8().data();
}
