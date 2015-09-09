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


static bool is_writable(const string &path) {
  return QFileInfo(path.c_str()).isWritable();
}


string FileDialog::open(const string &title, const string &filters,
                        const string &filename, bool save) {
  setWindowTitle(QString::fromLatin1(title.c_str()));
  setNameFilter(QString::fromLatin1(filters.c_str()));
  setAcceptMode(save ? AcceptSave : AcceptOpen);
  setConfirmOverwrite(save);

  string cwd = SystemUtilities::getcwd();
  string dir;

  if (filename.empty()) {
    selectFile(QString());
    dir = cwd;

  } else if (SystemUtilities::isDirectory(filename)) {
    selectFile(QString());
    dir = filename;

  } else {
    selectFile(SystemUtilities::basename(filename).c_str());
    dir = SystemUtilities::dirname(filename);
  }

  if (is_writable(dir)) setDirectory(dir.c_str());
  else if (is_writable(cwd)) setDirectory(cwd.c_str());
  else setDirectory(QDir::homePath());

  if (exec() != Accepted) return "";

  QStringList names = selectedFiles();
  if (!names.size()) return "";
  string path = names.at(0).toLatin1().data();

  if (SystemUtilities::isDirectory(path)) {
    win.warning("Cannot open directory");
    return "";
  }

  return path;
}
