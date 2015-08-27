/******************************************************************************\

    CAMotics is an Open-Source CAM software.
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

#include "FileDialog.h"

#include "QtWin.h"

#include <cbang/os/SystemUtilities.h>
#include <cbang/log/Logger.h>

#include <QString>

using namespace CAMotics;
using namespace cb;
using namespace std;


FileDialog::FileDialog(QtWin &win) : QFileDialog(&win), win(win) {
  setResolveSymlinks(false);
}


string FileDialog::open(const string &title, const string &filters,
                        const string &filename, bool save) {
  setWindowTitle(QString::fromAscii(title.c_str()));
  setNameFilter(QString::fromAscii(filters.c_str()));
  setAcceptMode(save ? AcceptSave : AcceptOpen);
  setConfirmOverwrite(save);

  if (filename.empty()) {
    selectFile(QString());
    setDirectory(SystemUtilities::getcwd().c_str());

  } else if (SystemUtilities::isDirectory(filename)) {
    selectFile(QString());
    setDirectory(filename.c_str());

  } else {
    selectFile(SystemUtilities::basename(filename).c_str());
    setDirectory(SystemUtilities::dirname(filename).c_str());
  }

  if (exec() != Accepted) return "";

  QStringList names = selectedFiles();
  if (!names.size()) return "";
  string path = names.at(0).toAscii().data();

  if (SystemUtilities::isDirectory(path)) {
    win.warning("Cannot open directory");
    return "";
  }

  return path;
}
